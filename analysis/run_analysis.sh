
# GET JOBID FROM ARGUMENTS
JOBID=$(expr $1 + $2)

SEED=$RANDOM$RANDOM

if test -z "${_CONDOR_SCRATCH_DIR}"; then
export _CONDOR_SCRATCH_DIR=/tmp/$USERNAME/mytmpdir/
fi
mkdir -p ${_CONDOR_SCRATCH_DIR}

PMTINFO=/net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/data/kpipe/PMTINFO.root

cd ${_CONDOR_SCRATCH_DIR}

# SETUP ENVIONMENT
source /net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/env.sh

# COPY EXE
cp /net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/analysis/analyze_data ./

# RUN RAT
#./analyze_data /net/nudsk0001/d00/scratch/taritree/kpipe_out/output_kpipe_"$JOBID".root output_analysis_"$JOBID".root ${PMTINFO}
./analyze_data /net/nudsk0001/d00/scratch/taritree/cosmic_events/output_kpipe_cryevents_"$JOBID".root output_cr_analysis_"$JOBID".root ${PMTINFO}

# COPY OUTPUT
#scp ${_CONDOR_SCRATCH_DIR}/output_analysis_* nudsk0001:/net/nudsk0001/d00/scratch/taritree/trg_out 2>> /net/hisrv0001/home/taritree/outlog
scp ${_CONDOR_SCRATCH_DIR}/output_cr_analysis_* nudsk0001:/net/nudsk0001/d00/scratch/taritree/cr_trg_out 2>> /net/hisrv0001/home/taritree/outlog
rm -rf ${_CONDOR_SCRATCH_DIR}

rm ${_CONDOR_SCRATCH_DIR}/*
