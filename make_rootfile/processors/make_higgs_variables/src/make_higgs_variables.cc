/**
*    @author Jonas Kunath, LLR, CNRS, École Polytechnique, IPP.
*/
// -- C++ STL headers.

// -- ROOT headers.
#include "TMath.h"
#include "Math/Vector4D.h"

// -- LCIO headers.
#include "EVENT/LCCollection.h"
#include "EVENT/ParticleID.h"
#include "EVENT/ReconstructedParticle.h"
#include "UTIL/PIDHandler.h"

// -- Marlin headers.
#include "marlin/Exceptions.h"

// -- Header for this processor and other project-specific headers.
#include "make_higgs_variables.h"

// -- Using-declarations and global constants.
// Only in .cc files, never in .h header files!
using RP = EVENT::ReconstructedParticle;
using Tlv = ROOT::Math::XYZTVector;

// This line allows to register your processor in marlin when calling
// "Marlin steering_file.xml".
MakeHiggsVariablesProcessor aMakeHiggsVariablesProcessor;

// ----------------------------------------------------------------------------
MakeHiggsVariablesProcessor::MakeHiggsVariablesProcessor() :
    marlin::Processor("MakeHiggsVariablesProcessor") {
  _description = "Build a .root file of the format necessary for my master "
    "thesis analysis.";

  registerInputCollection(
    LCIO::RECONSTRUCTEDPARTICLE,
    "HiggsCollection",
    "Name of the new PFO collection with (only) the Higgs decay remnants.",
    higgs_only_collection_name_,
    std::string("PandoraPFOs"));

  registerInputCollection(
    LCIO::MCPARTICLE,
    "MCParticleCollection",
    "MCParticle collection for Higgs decay truth information.",
    mc_collection_name,
    std::string("MCParticlesSkimmed"));

  registerProcessorParameter(
    "OutputRootFile",
    "Name of the output root file.",
    root_file_name_,
    std::string("higgs_variables"));
}

// ----------------------------------------------------------------------------

void MakeHiggsVariablesProcessor::initRoot() {
  const char* tree_name = "higgs";
  tree_ = new TTree(tree_name, "Input for the Higgs BR classes.");
  tv.initBranches(tree_);
}

void MakeHiggsVariablesProcessor::endRoot() {
  TString fnn(root_file_name_.c_str()); fnn += ".root";
  root_file_ = new TFile(fnn, "update");
  root_file_->cd();
  TTree* tree_in_write_file = tree_->CloneTree();
  tree_in_write_file->Write();
  root_file_->Write();
  root_file_->Close();
}

void MakeHiggsVariablesProcessor::init() {
  initRoot();
}

void MakeHiggsVariablesProcessor::end() {
  endRoot();
  if (missing_mc_collection) {
    streamlog_out(ERROR) << "At least one event did not provide "
      << "a MC Collection named " << mc_collection_name << ". " << std::endl;
  }
}

// ----------------------------------------------------------------------------
void MakeHiggsVariablesProcessor::processEvent(EVENT::LCEvent* event) {
  streamlog_out(DEBUG) << "Processing event no " << event->getEventNumber()
    << std::endl;
  tv.resetValues();

  setHiggsKinematicInfo(event);
  setIsolatedNumbers(event);
  tv.higgs_truth = getHiggsTruth(event);
  tree_->Fill();
}


// ----------------------------------------------------------------------------
void MakeHiggsVariablesProcessor::setHiggsKinematicInfo(EVENT::LCEvent* event) {
  EVENT::LCCollection* higgs_collection = nullptr;
  try {
    higgs_collection = event->getCollection(higgs_only_collection_name_);
  } catch (DataNotAvailableException &e) {
    streamlog_out(ERROR) << "RP collection " << higgs_only_collection_name_
      << " is not available!" << std::endl;
    throw marlin::StopProcessingException(this);
  }
  Tlv higgs_four_vector(0, 0, 0, 0);
  for (int i = 0; i < higgs_collection->getNumberOfElements(); ++i) {
    RP* rp = static_cast<RP*>(higgs_collection->getElementAt(i));
    Tlv rp_four_vector = Tlv(rp->getMomentum()[0], rp->getMomentum()[1],
      rp->getMomentum()[2], rp->getEnergy());
    higgs_four_vector += rp_four_vector;
    if (fabs(cos(rp_four_vector.Theta())) < 0.95) tv.n_pfos_not_forward++;

    int abs_pdg = abs(rp->getType());
    if (abs_pdg == 11) {
      tv.n_electrons += 1;
    } else if (abs_pdg == 22) {
      tv.n_gamma += 1;
    } else if (abs_pdg == 13) {
      tv.n_muons += 1;
    } else if (abs_pdg == 211 || abs_pdg == 321 || abs_pdg == 2212) {
      tv.n_charged_hadrons += 1;
    } else if (abs_pdg == 130 || abs_pdg == 310 || abs_pdg == 2112 ||
               abs_pdg == 3122) {
      tv.n_neutral_hadrons += 1;
    } else {
      streamlog_out(WARNING) << "An unexpected PDG was found: " << abs_pdg
        << "." << std::endl;
    }
  }
  tv.n_pfos = higgs_collection->getNumberOfElements();

  tv.e_h = higgs_four_vector.E();
  tv.m_h = higgs_four_vector.M();
  tv.m_h_recoil = (Tlv(0, 0, 0, 250) - higgs_four_vector).mass();
  tv.cos_theta_miss = cos(higgs_four_vector.Theta());
}


// ----------------------------------------------------------------------------
void MakeHiggsVariablesProcessor::setIsolatedNumbers(EVENT::LCEvent* event) {
  //
  EVENT::LCCollection* lepton_collection = nullptr;
  try {
    lepton_collection = event->getCollection("IsolatedLeptons");
  } catch (DataNotAvailableException &e) {
    streamlog_out(ERROR) << "RP collection " << "IsolatedLeptons"
      << " is not available! Remember calling the IsoLeptonTagging "
      "Processor before this one." << std::endl;
    throw marlin::StopProcessingException(this);
  }

  IntVec tagged_lepton_types;
  lepton_collection->getParameters().getIntVals(
      "ISOLepType", tagged_lepton_types);

  tv.n_isolated_leptons = lepton_collection->getNumberOfElements();
}
