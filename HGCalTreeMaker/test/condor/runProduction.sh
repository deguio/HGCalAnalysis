#!/bin/sh

JOB=${1}
NOISE=${2}
ALGO=${3}
SCA=${4}
SCD=${5}
MAXEVENTS=${6}

BASENAME=${NOISE}_${ALGO}_${SCA}_${SCD}
BASENAME=$(echo ${BASENAME} | sed -e s%"="%"_"%g)
WORKDIR=`pwd`

echo "Job number is ${JOB} will generate ${MAXEVENTS} events"
echo "Work dir: ${WORKDIR} at `hostname`"

echo $BASENAME


outDir=/eos/cms/store/group/dpg_hgcal/comm_hgcal/deguio/gen_mu150_scan_highStat_randSeed/
mkdir -p ${outDir}

cd /afs/cern.ch/work/d/deguio/HGCAL/DigiStudies/CMSSW_10_6_0_pre2_digiDev/src/HGCalAnalysis/HGCalTreeMaker/test/condor/
eval `scramv1 runtime -sh`
cd ${WORKDIR}

cmsRun $CMSSW_BASE/src/HGCalAnalysis/HGCalTreeMaker/test/mugun_HEback.py ${NOISE} ${ALGO} ${SCA} ${SCD} ${MAXEVENTS}
mv *.root ${outDir}/${BASENAME}_${JOB}.root
