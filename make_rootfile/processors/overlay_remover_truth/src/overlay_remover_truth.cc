/**
*    @author Jonas Kunath, LLR, CNRS, École Polytechnique, IPP.
*/
// -- C++ STL headers.

// -- ROOT headers.

// -- LCIO headers.
#include "EVENT/LCCollection.h"
#include "IMPL/LCCollectionVec.h"

// -- Marlin headers.
#include "marlin/Exceptions.h"

// -- Header for this processor and other project-specific headers.
#include "overlay_remover_truth.h"

// -- Using-declarations and global constants.
// Only in .cc files, never in .h header files!
using RP = EVENT::ReconstructedParticle;
using MCP = EVENT::MCParticle;


// This line allows to register your processor in marlin when calling
// "Marlin steering_file.xml".
OverlayRemoverTruthProcessor aOverlayRemoverTruthProcessor;

// ----------------------------------------------------------------------------
OverlayRemoverTruthProcessor::OverlayRemoverTruthProcessor() :
    marlin::Processor("OverlayRemoverTruthProcessor") {
  _description = "Build a .root file of the format necessary for my master "
    "thesis analysis.";

  registerInputCollection(
    LCIO::RECONSTRUCTEDPARTICLE,
    "PfoCollection",
    "The Pandora PFO collection name.",
    full_pfo_collection_name_,
    std::string("PandoraPFOs"));

  registerOutputCollection(
    LCIO::RECONSTRUCTEDPARTICLE,
    "HiggsOnlyCollection",
    "Name of the new PFO collection with (only) the Higgs decay remnants.",
    higgs_only_collection_name_,
    std::string("HiggsOnly"));
}

// ----------------------------------------------------------------------------
void OverlayRemoverTruthProcessor::processEvent(EVENT::LCEvent* event) {
  streamlog_out(DEBUG) << "Processing event no " << event->getEventNumber()
    << std::endl;

  EVENT::LCCollection* full_collection = nullptr;
  try {
    full_collection = event->getCollection(full_pfo_collection_name_);
  } catch (DataNotAvailableException &e) {
    streamlog_out(ERROR) << "RP collection " << full_pfo_collection_name_
      << " is not available!" << std::endl;
    throw marlin::StopProcessingException(this);
  }

  std::vector<RP*> not_overlay_particles;
  for (int e = 0; e < full_collection->getNumberOfElements(); ++e) {
    RP* pfo = static_cast<RP*>(full_collection->getElementAt(e));
    if (isFromHiggs(pfo, event)) not_overlay_particles.push_back(pfo);
  }

  LCCollectionVec* not_overlay_vec = new LCCollectionVec(
      LCIO::RECONSTRUCTEDPARTICLE);
  not_overlay_vec->setSubset(true);
  for (auto pfo : not_overlay_particles) not_overlay_vec->addElement(pfo);
  event->addCollection(not_overlay_vec, higgs_only_collection_name_.c_str());
}

bool OverlayRemoverTruthProcessor::isFromHiggs(
    RP* rp, UTIL::LCRelationNavigator* relation_navigator) {
  int higgs_pdg = 25;
  if (relation_navigator->getRelatedToObjects(rp).size() == 0) {
    std::cout << "There is a ReconstructedParticle that is not related to any"
      << " MonteCarlo particle. Maybe the relation collection is faulty?"
      << std::endl;
    return false;
  }
  MCP* mcp = static_cast<MCP*>(relation_navigator->getRelatedToObjects(rp)[0]);
  std::vector<MCP*> mc_parents = mcp->getParents();
  while (mc_parents.size() > 0) {
    mcp = mc_parents.back();
    mc_parents.pop_back();
    if (mcp->getPDG() == higgs_pdg) return true;
    for (auto parent : mcp->getParents()) {
      mc_parents.push_back(parent);
    }
  }
  return false;
}

bool OverlayRemoverTruthProcessor::isFromHiggs(RP* rp, EVENT::LCEvent* event) {
  std::string link_col_name = "RecoMCTruthLink";
  EVENT::LCCollection* relation_collection = nullptr;
  UTIL::LCRelationNavigator* relation_navigator = nullptr;
  try {
    relation_collection = event->getCollection(link_col_name);
    relation_navigator = new UTIL::LCRelationNavigator(relation_collection);
  } catch (DataNotAvailableException &e) {
    streamlog_out(ERROR) << "The relation collection " << link_col_name
      << " is not available!"
      << "false is returned for this search." << std::endl;
    return false;
  }
  return isFromHiggs(rp, relation_navigator);
}
