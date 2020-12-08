/**
*    @author Jonas Kunath, LLR, CNRS, École Polytechnique, IPP.
*/
// -- C++ STL headers.

// -- ROOT headers.

// -- LCIO headers.
#include "EVENT/LCCollection.h"
#include "EVENT/MCParticle.h"

// -- Marlin headers.
#include "marlin/Exceptions.h"

// -- Header for this processor and other project-specific headers.
#include "make_higgs_variables.h"

// -- Using-declarations and global constants.
// Only in .cc files, never in .h header files!
using MCP = EVENT::MCParticle;

// ----------------------------------------------------------------------------
bool isHiggsToSameParticlePair(std::vector<MCP*> remnants) {
  if (remnants.size() != 2) return false;
  return fabs(remnants[0]->getPDG()) == fabs(remnants[1]->getPDG());
}

bool isHiggsToZGamma(std::vector<MCP*> r) {
  if (r.size() != 2) return false;
  if ((fabs(r[0]->getPDG()) == 22)  & (fabs(r[1]->getPDG()) == 23)) return true;
  if ((fabs(r[0]->getPDG()) == 23)  & (fabs(r[1]->getPDG()) == 22)) return true;
  return false;
}

MakeHiggsVariablesProcessor::HiggsTruth MakeHiggsVariablesProcessor::getHiggsTruth(
    EVENT::LCEvent* event
  ) {
  HiggsTruth higgs_info;

  EVENT::LCCollection* mc_collection = nullptr;
  try {
    mc_collection = event->getCollection(mc_collection_name);
  } catch (DataNotAvailableException &e) {
    missing_mc_collection = true;
    return higgs_info;
  }

  for (int i = 0; i < mc_collection->getNumberOfElements(); ++i) {
    MCP* mcp = static_cast<MCP*>(mc_collection->getElementAt(i));
    bool is_higgs = mcp->getPDG() == 25;
    if (!is_higgs) continue;

    std::vector<MCP*> remnants = mcp->getDaughters();
    bool is_intermediate_higgs = remnants[0]->getPDG() == 25;  // E.g. from hadronization.
    if (is_intermediate_higgs) continue;

    if (isHiggsToSameParticlePair(remnants)) {
      higgs_info.decay_mode = fabs(remnants[0]->getPDG());
    } else if (isHiggsToZGamma(remnants)) {
      higgs_info.decay_mode = 20;
    } else {
      streamlog_out(ERROR) << "An unforeseen Higgs decay occurred. "
        << "The decay prodcuts are: ";
      for (auto r : remnants) streamlog_out(ERROR) << r->getPDG() << ", ";
      streamlog_out(ERROR) << "." << std::endl;
    }

    higgs_info.decays_invisible = decaysInvisible(remnants);
    higgs_info.n_jets = getNTrueJets(remnants);
  }
  return higgs_info;
}


bool MakeHiggsVariablesProcessor::decaysInvisible(std::vector<MCP*> remnants) {
  std::vector<MCP*> stable_higgs_remnants;
  while (!remnants.empty()) {
    MCP* decay_product = remnants.back();
    remnants.pop_back();
    if (decay_product->getGeneratorStatus() == 1) {
      stable_higgs_remnants.push_back(decay_product);
    } else {
      for (auto daughter : decay_product->getDaughters()) {
        remnants.push_back(daughter);
      }
    }
  }
  bool decays_invisible = true;
  for (auto stable_mcp : stable_higgs_remnants) {
    int abs_pdg = fabs(stable_mcp->getPDG());
    bool is_visible = (abs_pdg != 12) && (abs_pdg != 14) && (abs_pdg != 16);
    if (is_visible) {
      decays_invisible = false;
      break;
    }
  }
  return decays_invisible;
}


bool isIn(std::vector<int> integers, int candidate) {
  auto find_position = std::find(integers.begin(), integers.end(), candidate);
  return find_position != integers.end();
}

bool isLeptonicTauDecay(MCP* tau) {
  for (MCP* tau_daughter : tau->getDaughters()) {
    int d_pdg = abs(tau_daughter->getPDG());
    if ((d_pdg == 11) || (d_pdg == 13)) return true;
    if ((d_pdg == 12) || (d_pdg == 14)) continue;
    if ((d_pdg == 15) || (d_pdg == 24)) return isLeptonicTauDecay(tau_daughter);
    if (d_pdg == 16) continue;  // Can appear in hadronization.
    if (d_pdg == 94) {  // The hadronization PDG code.
      int tau_pdg = tau->getPDG();
      for (MCP* mc_after_mixing : tau_daughter->getDaughters()) {
        if (mc_after_mixing->getPDG() == tau_pdg) {
          return isLeptonicTauDecay(mc_after_mixing);
        }
      }
      // Should never reach here.
      streamlog_out(ERROR) << "Tau-daughters: ";
      for (MCP* mc_after_mixing : tau_daughter->getDaughters()) {
        streamlog_out(ERROR) << mc_after_mixing->getPDG() << ", ";
      }
      streamlog_out(ERROR) << std::endl;
      return false;
    }
    return false;
  }
  return false;
}

int MakeHiggsVariablesProcessor::getNTrueJets(std::vector<MCP*> remnants) {
  int n_true_jets = 0;
  // quarks, gluon and tau "jets".
  std::vector<int> jet_forming_pdgs = {1, 2, 3, 4, 5, 6, 21};
  // No jet: Invisible neutrinos, or isolated photons and charged leptons.
  std::vector<int> no_jet_pdgs = {11, 13, 12, 14, 16, 22};
  while (!remnants.empty()) {
    MCP* decay_product = remnants.back();
    remnants.pop_back();

    int pdg = fabs(decay_product->getPDG());
    if (isIn(jet_forming_pdgs, pdg)) {
      n_true_jets++;
    } else if (isIn(no_jet_pdgs, pdg)) {
      continue;
    } else if (pdg == 15) {
      if (isLeptonicTauDecay(decay_product)) continue;
      n_true_jets++;
    } else {
      if ((decay_product->getGeneratorStatus() == 1 ) &&
          (decay_product->getDaughters().size() == 0)) {
        streamlog_out(ERROR) << "This particle has neither daughters, nor is it"
          << " identified as a (stable) lepton/quark leading to a jet "
          << "or a neutrino. PDG:" << pdg << std::endl;
      }
      for (auto daughter : decay_product->getDaughters()) {
        if (std::find(remnants.begin(), remnants.end(), daughter)
            == remnants.end()) {
          remnants.push_back(daughter);
        }
      }
    }
  }
  return n_true_jets;
}