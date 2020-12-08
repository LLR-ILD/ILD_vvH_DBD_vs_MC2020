/**
 *  Exploit MC truth information to remove beam overlay (low pT hadrons).
 *
 *  This first version only works on nnH events: It removes anything that does
 *  not origin from the Higgs boson itself.
 *
 *    @author Jonas Kunath, LLR, CNRS, École Polytechnique, IPP.
 */
#ifndef _MAKE_HIGGS_VARIABLES_PROCESSOR_H_
#define _MAKE_HIGGS_VARIABLES_PROCESSOR_H_
// -- C++ STL headers.

// -- ROOT headers.
#include "TFile.h"
#include "TTree.h"

// -- LCIO headers.
#include "EVENT/LCEvent.h"
#include "EVENT/MCParticle.h"
#include "EVENT/ReconstructedParticle.h"
#include "UTIL/LCRelationNavigator.h"

// -- Marlin headers.
#include "marlin/Processor.h"

// -- Header for this processor and other project-specific headers.

class OverlayRemoverTruthProcessor : public marlin::Processor {
 public:
  marlin::Processor* newProcessor() { return new OverlayRemoverTruthProcessor(); }
  OverlayRemoverTruthProcessor();

  // These two lines avoid frequent compiler warnings when using -Weffc++.
  OverlayRemoverTruthProcessor(const OverlayRemoverTruthProcessor&) = delete;
  OverlayRemoverTruthProcessor& operator=(const OverlayRemoverTruthProcessor&) = delete;
  bool isFromHiggs(ReconstructedParticle* rp, EVENT::LCEvent* event);
  bool isFromHiggs(ReconstructedParticle* rp,
                    UTIL::LCRelationNavigator* relation_navigator);
  void processEvent(EVENT::LCEvent* event);

 private:
  // -- Parameters registered in steering file.
  std::string full_pfo_collection_name_{""};
  std::string higgs_only_collection_name_{""};
  std::string mc_collection_name{""};
};
#endif