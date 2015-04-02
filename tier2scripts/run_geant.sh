
# GET JOBID FROM ARGUMENTS
JOBID=$1

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

# COPY EXE
cp /net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/bin/rat ./
# GEN MACRO
python /net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/tier2scripts/gen_macro.py kpipe_job"$JOBID".mac

./rat kpipe_job"$JOBID".mac -o output_kpipe_"$SEED".root

i=`tail -n 1 /net/hisrv0001/home/taritree/batch_runlist_geant|awk -F'[_]' '{print $2}'`
tail -n 1 /net/hisrv0001/home/taritree/batch_runlist_geant
exit=`echo $?`

if [ "$i" == ""  ];
then
i=10000
fi

let ii=$i+1

mv output_kpipe_"$SEED".root output_kpipe_"$ii".root 
echo run_"$ii"_seed_"$SEED">>/net/hisrv0001/home/taritree/batch_runlist_geant


scp ${_CONDOR_SCRATCH_DIR}/output_kpipe_* nudsk0001:/net/nudsk0001/d00/scratch/taritree/kpipe_out 2>> /net/hisrv0001/home/taritree/outlog
rm -rf ${_CONDOR_SCRATCH_DIR}

rm ${_CONDOR_SCRATCH_DIR}/*