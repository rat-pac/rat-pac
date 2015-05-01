
# GET JOBID FROM ARGUMENTS
JOBID=$(expr $1 + $2)

SEED=$RANDOM$RANDOM

if test -z "${_CONDOR_SCRATCH_DIR}"; then
export _CONDOR_SCRATCH_DIR=/tmp/$USERNAME/mytmpdir/
fi
mkdir -p ${_CONDOR_SCRATCH_DIR}


cd ${_CONDOR_SCRATCH_DIR}
#echo $PWD>>/net/hisrv0001/home/spitzj/outlog

# SETUP ENVIONMENT
source /net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/env.sh

#mkdir -p out

# input file
INPUTFILE=/net/nudsk0001/d00/scratch/taritree/cry_gen/cry_events_"$JOBID".root

# COPY FILES
cp /net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/bin/rat ./
cp ${INPUTFILE} ./

# GEN MACRO
python /net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/cry/gen_macro.py cry_events_"$JOBID".root cry_job"$JOBID".mac $JOBID 5000

# RUN JOB
./rat cry_job"$JOBID".mac -o output_kpipe_cryevents_"$JOBID".root 2>> /net/hisrv0001/home/taritree/outlog

# COPY OUTPUT
scp ${_CONDOR_SCRATCH_DIR}/output_kpipe_cryevents_* nudsk0001:/net/nudsk0001/d00/scratch/taritree/cosmic_events 2>> /net/hisrv0001/home/taritree/outlog
rm -rf ${_CONDOR_SCRATCH_DIR}

rm ${_CONDOR_SCRATCH_DIR}/*
