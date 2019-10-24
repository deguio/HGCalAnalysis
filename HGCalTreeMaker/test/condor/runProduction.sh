#!/bin/sh

JOB=${1}
NOISE=${2}
ALGO=${3}
PILEUP=${4}
SCA=${5}
MAXEVENTS=${6}
MOMENTUM=${7}

BASENAME=${NOISE}_${ALGO}_${PILEUP}_${SCA}_${MOMENTUM}
BASENAME=$(echo ${BASENAME} | sed -e s%"="%"_"%g)
WORKDIR=`pwd`

echo "Job number is ${JOB} will generate ${MAXEVENTS} events"
echo "Work dir: ${WORKDIR} at `hostname`"

echo $BASENAME


outDir=/eos/cms/store/group/dpg_hgcal/comm_hgcal/deguio/gen_mu_pscan_20191021/
mkdir -p ${outDir}

cd /afs/cern.ch/work/d/deguio/HGCAL/DigiStudies/CMSSW_11_0_0_pre10_drawBB/src/HGCalAnalysis/HGCalTreeMaker/test/condor
eval `scramv1 runtime -sh`
cd ${WORKDIR}

cmsRun $CMSSW_BASE/src/HGCalAnalysis/HGCalTreeMaker/test/pgun_local.py ${NOISE} ${ALGO} ${PILEUP} ${SCA} ${MAXEVENTS} ${MOMENTUM}
mv *.root ${outDir}/${BASENAME}_${JOB}.root
