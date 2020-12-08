from pysteer import Pysteer, lcio_file_dict

from pathlib import Path


def getJobDict(processes, files_per_job=10, machine="E250-TDR_ws"):
    new_files = lcio_file_dict(machine=machine)
    jobs = {}
    for pol, process in list(new_files.items()):
        for process in processes:
            if process not in new_files[pol]: continue
            unsplit_f = new_files[pol][process]
            for n_job, i in enumerate(range(0, len(unsplit_f), files_per_job)):
                job_name = f"{pol}_{process}_{n_job:02}"
                jobs[job_name] = unsplit_f[i:i+files_per_job]
    return jobs


def newSampleRun(steerer):
    jobs = getJobDict(["Pn1n1h", "Pn23n23h"], machine="E250-SetA",
                      files_per_job=5)
    # jobs = {k: v for k,v in list(jobs.items())[:2]}  # For debugging.
    print(f"{len(jobs)} jobs will be submitted to the queue.")
    steerer.marlin_global.MaxRecordNumber = -1  # Per job.
    steerer.run(batch_mode=True, batch_processes=jobs)


def oldSampleRun(steerer):
    jobs = getJobDict(["Pnnh"], machine="E250-TDR_ws",
                      files_per_job=5)
    print(f"{len(jobs)} jobs will be submitted to the queue.")
    steerer.run(batch_mode=True, batch_processes=jobs)

# ------------------------------------------------------------------------------
if __name__ == "__main__":
    cpp_folder = Path(__file__).resolve().parent.parent
    steerer = Pysteer(
        local_project_paths=[cpp_folder],
        ilcsoft_processors=[
            "InitializeDD4hep",
            "IsolatedLeptonTaggingProcessor",
            "LCIOOutputProcessor",
            "LcfiplusProcessor"]
    )
    steerer.marlin_global.Verbosity = "MESSAGE"


    steerer.add("IsolatedLeptonTaggingProcessor", {
        "DirOfISOElectronWeights": dict(value="/cvmfs/ilc.desy.de/sw/x86_64_gcc49_sl6/v02-00-02/MarlinReco/v01-25/Analysis/IsolatedLeptonTagging/weights/e1e1h_gg_qqqq_250"),
        "DirOfISOMuonWeights":     dict(value="/cvmfs/ilc.desy.de/sw/x86_64_gcc49_sl6/v02-00-02/MarlinReco/v01-25/Analysis/IsolatedLeptonTagging/weights/yyxylv_yyxyyx_woYoke_500.mILD_l5_o1_v02"),
        "IsSelectingOneIsoLep": dict(value="false"),
        "InputPandoraPFOsCollection": dict(value="PandoraPFOs"),
        "OutputIsoLeptonsCollection": dict(value="IsolatedLeptons"),
        "OutputPFOsWithoutIsoLepCollection": dict(value="PFOsNoIsoleptons"),
    })
    steerer.add("MakeHiggsVariablesProcessor", {
        "HiggsCollection": dict(value="PandoraPFOs"),
        "MCParticleCollection": dict(value="MCParticlesSkimmed"),
        "OutputRootFile": dict(value="higgs_variables"),
    })
    steerer.add("OverlayRemoverTruthProcessor", {
        "PfoCollection": dict(value="PandoraPFOs"),
        "HiggsOnlyCollection": dict(value="HiggsOnly"),
    })
    steerer.add("MakeHiggsVariablesProcessor", {
        "HiggsCollection": dict(value="HiggsOnly"),
        "MCParticleCollection": dict(value="MCParticlesSkimmed"),
        "OutputRootFile": dict(value="no_overlay_higgs_variables"),
    })

    newSampleRun(steerer)
    oldSampleRun(steerer)
