
# GET JOBID FROM ARGUMENTS
JOBID=$(expr $1 + $2)

SEED=$RANDOM$RANDOM

if test -z "${_CONDOR_SCRATCH_DIR}"; then
export _CONDOR_SCRATCH_DIR=/tmp/$USERNAME/mytmpdir/
fi
mkdir -p ${_CONDOR_SCRATCH_DIR}


cd ${_CONDOR_SCRATCH_DIR}

# SETUP ENVIONMENT
source /net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/env.sh

# COPY EXE
cp /net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/cry/gen_cosmics_kpipe ./
cp /net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/cry/setup.file ./

./gen_cosmics_kpipe setup.file cry_events_"$JOBID".root 10000 "$JOBID" 2>> /net/hisrv0001/home/taritree/outlog

# COPY OUTPUT
scp ${_CONDOR_SCRATCH_DIR}/cry_events_* nudsk0001:/net/nudsk0001/d00/scratch/taritree/cry_gen/ 2>> /net/hisrv0001/home/taritree/outlog
rm -rf ${_CONDOR_SCRATCH_DIR}

rm ${_CONDOR_SCRATCH_DIR}/*
