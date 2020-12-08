/**
 *  This processor can (only) be called after some preparatory steps:
 *    - The IsolatedLeptonTagging processor must have been run.
 *
 *  It produces a .root file that then can readily be put into the uproot-python
 *  analysis chain.
 *
 *  The entries are simple observables.
 *  The processor was created in an attempt to get used to the new ILD 2020 MC
 *  samples and understand possible difference wrt. the DBD samples.
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
#include "EVENT/MCParticle.h"

// -- Marlin headers.
#include "marlin/Processor.h"

// -- Header for this processor and other project-specific headers.

class MakeHiggsVariablesProcessor : public marlin::Processor {
 public:
  marlin::Processor* newProcessor() { return new MakeHiggsVariablesProcessor(); }
  MakeHiggsVariablesProcessor();

  // These two lines avoid frequent compiler warnings when using -Weffc++.
  MakeHiggsVariablesProcessor(const MakeHiggsVariablesProcessor&) = delete;
  MakeHiggsVariablesProcessor& operator=(const MakeHiggsVariablesProcessor&) = delete;

  void init();
  void processEvent(EVENT::LCEvent* event);
  void end();

  void initRoot();
  void endRoot();
  void setHiggsKinematicInfo(EVENT::LCEvent* event);
  void setIsolatedNumbers(EVENT::LCEvent* event);
  void evaluateLCFIPlus(EVENT::LCEvent* event);

 private:
  // -- Parameters registered in steering file.
  std::string higgs_only_collection_name_{""};
  std::string mc_collection_name{""};
  std::string flavor_tagged_collection_name{""};

  // -- The root file
  TFile* root_file_{};
  std::string root_file_name_ = {""};
  TTree* tree_ = {};

  bool missing_mc_collection = false;
  struct HiggsTruth {
    int decays_invisible = false;
    int decay_mode = -1;
    int n_jets = -1;
  };
  HiggsTruth getHiggsTruth(EVENT::LCEvent* event);
  bool decaysInvisible(std::vector<EVENT::MCParticle*> remnants);
  int getNTrueJets(std::vector<EVENT::MCParticle*> remnants);

  struct TreeVars {
    TreeVars() {higgs_truth = HiggsTruth();};
    ~TreeVars() {};

    int n_isolated_leptons = -1;
    int n_pfos = -1;
    int n_pfos_not_forward = -1;
    int n_charged_hadrons = -1;
    int n_neutral_hadrons = -1;
    int n_gamma = -1;
    int n_electrons = -1;
    int n_muons = -1;

    float e_h = -1;
    float m_h = -1;
    float m_h_recoil = -1;
    float cos_theta_miss = -1;

    HiggsTruth higgs_truth{};

    void initBranches(TTree* tree) {
      tree->Branch(("n_isolated_leptons"), &n_isolated_leptons, ("n_isolated_leptons/I"));
      tree->Branch(("n_pfos"), &n_pfos, ("n_pfos/I"));
      tree->Branch(("n_pfos_not_forward"), &n_pfos_not_forward, ("n_pfos_not_forward/I"));
      tree->Branch(("n_charged_hadrons"), &n_charged_hadrons, ("n_charged_hadrons/I"));
      tree->Branch(("n_neutral_hadrons"), &n_neutral_hadrons, ("n_neutral_hadrons/I"));
      tree->Branch(("n_gamma"), &n_gamma, ("n_gamma/I"));
      tree->Branch(("n_electrons"), &n_electrons, ("n_electrons/I"));
      tree->Branch(("n_muons"), &n_muons, ("n_muons/I"));

      tree->Branch(("e_h"), &e_h, ("e_h/F"));
      tree->Branch(("m_h"), &m_h, ("m_h/F"));
      tree->Branch(("m_h_recoil"), &m_h_recoil, ("m_h_recoil/F"));
      tree->Branch(("cos_theta_miss"), &cos_theta_miss, ("cos_theta_miss/F"));

      tree->Branch(("h_invisible"), &higgs_truth.decays_invisible, ("h_invisible/I"));
      tree->Branch(("h_decay"), &higgs_truth.decay_mode, ("h_decay/I"));
    }

    void resetValues() {
      n_isolated_leptons = 0;
      n_pfos = 0;
      n_pfos_not_forward = 0;
      n_charged_hadrons = 0;
      n_neutral_hadrons = 0;
      n_gamma = 0;
      n_electrons = 0;
      n_muons = 0;

      e_h = 0;
      m_h = 0;
      m_h_recoil = 0;
      cos_theta_miss = 0;

      higgs_truth = HiggsTruth();
    }
  };
  TreeVars tv{};
};
#endif