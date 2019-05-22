#!/bin/sh

JOB=${1}
NOISE=${2}
ALGO=${3}
PILEUP=${4}
SCA=${5}
MAXEVENTS=${6}

BASENAME=${NOISE}_${ALGO}_${PILEUP}_${SCA}
BASENAME=$(echo ${BASENAME} | sed -e s%"="%"_"%g)
WORKDIR=`pwd`

echo "Job number is ${JOB} will generate ${MAXEVENTS} events"
echo "Work dir: ${WORKDIR} at `hostname`"

echo $BASENAME


outDir=/eos/cms/store/group/dpg_hgcal/comm_hgcal/deguio/gen_mu150_20190515/
mkdir -p ${outDir}

cd /afs/cern.ch/work/d/deguio/HGCAL/DigiStudies/CMSSW_10_6_0_pre4_occStudies/src/HGCalAnalysis/HGCalTreeMaker/test/condor
eval `scramv1 runtime -sh`
cd ${WORKDIR}

cmsRun $CMSSW_BASE/src/HGCalAnalysis/HGCalTreeMaker/test/mugun_HEback.py ${NOISE} ${ALGO} ${PILEUP} ${SCA} ${MAXEVENTS}
mv *.root ${outDir}/${BASENAME}_${JOB}.root
