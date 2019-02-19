#ifndef HGCalTupleMaker_HGCDigis_h
#define HGCalTupleMaker_HGCDigis_h

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/transform.h"

// HGCAL objects
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "DataFormats/HGCDigi/interface/HGCDigiCollections.h"
#include "DataFormats/HcalDigi/interface/HBHEDataFrame.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/ForwardDetId/interface/HGCEEDetId.h"
#include "DataFormats/ForwardDetId/interface/HGCHEDetId.h"
#include "DataFormats/DetId/interface/DetId.h"

// Geometry
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"

//
#include "DataFormats/HcalDetId/interface/HcalDetId.h"

// HGCAL & HCAL Geometry
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"
#include "SimDataFormats/CaloTest/interface/HGCalTestNumbering.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "Geometry/HGCalCommonData/interface/HGCalDDDConstants.h"
#include "Geometry/HcalCommonData/interface/HcalDDDRecConstants.h"

#include "DataFormats/HcalDetId/interface/HcalTestNumbering.h"


#include "Geometry/HcalCommonData/interface/HcalHitRelabeller.h"

// Used:
// http://cmslxr.fnal.gov/source/AnalysisAlgos/SiStripClusterInfoProducer/plugins/SiStripProcessedRawDigiProducer.cc
// as an example

class HGCalTupleMaker_HGCDigis : public edm::EDProducer {
 protected:

  std::vector<edm::InputTag> m_HGCDigisTags;

  edm::EDGetTokenT<HGCalDigiCollection> m_HGCEEDigisToken;
  edm::EDGetTokenT<HGCalDigiCollection> m_HGCHEDigisToken;
  edm::EDGetTokenT<HGCalDigiCollection> m_HGCBHDigisToken;
  std::vector<edm::EDGetTokenT<HGCalDigiCollection> > m_HGCDigisTokens;

  std::vector<std::string> m_geometrySource;

  const std::string     m_prefix;
  const std::string     m_suffix;

  int SampleIndx;
  
  bool debug=false;
  bool debug_geom=false;
  bool detid_store=true;
  
  //HGC Geometry                                                                                                     
  std::vector<const HGCalDDDConstants*> hgcCons_;
  std::vector<const HGCalGeometry*>     hgcGeometry_;
  const HcalDDDSimConstants*            hcCons_;
  const HcalDDDRecConstants*            hcConr_;
  const CaloSubdetectorGeometry*        hcGeometry_;
  
  void produce( edm::Event & iEvent, const edm::EventSetup & iSetup) { 

    //-----------------------------------------------------
    // Prepare to put things into event
    //-----------------------------------------------------
    
    loadAlgo();

    //-----------------------------------------------------
    // edm::Handles
    //-----------------------------------------------------
    
    edm::Handle<HGCalDigiCollection> HGCEEDigis;
    edm::Handle<HGCalDigiCollection> HGCHEDigis;
    edm::Handle<HGCalDigiCollection> HGCBHDigis;

    int geomType(0);
    
    // Loop over input tags
    for( typename std::vector<edm::InputTag>::const_iterator
	   tag = m_HGCDigisTags.begin(); tag != m_HGCDigisTags.end(); ++tag ) {

      unsigned index(tag - m_HGCDigisTags.begin());
      if (debug) std::cout << index << std::endl;

      std::string nameDetector_ = m_geometrySource[index];
      if (debug) std::cout << nameDetector_ << std::endl;

      edm::ESHandle<HGCalGeometry> geom;
      iSetup.get<IdealGeometryRecord>().get(nameDetector_, geom);
      if (!geom.isValid()) {
	edm::LogWarning("HGCalTupleMaker_HGCDigis")
	  << "Cannot get valid HGCalGeometry Object for " << nameDetector_;
	return;
      }
      const HGCalGeometry* geom0 = geom.product();

      geomType=0;
      HGCalGeometryMode::GeometryMode mode = geom0->topology().geomMode();
      if ((mode == HGCalGeometryMode::Hexagon8) ||
	  (mode == HGCalGeometryMode::Hexagon8Full)) geomType = 1;
      else if (mode == HGCalGeometryMode::Trapezoid) geomType = 2;

      HGCBHDigis.clear();
      iEvent.getByToken(m_HGCDigisTokens[index], HGCBHDigis);
	
      const HGCalDigiCollection* bhDigis = HGCBHDigis.product(); // get a ptr to the product
      for(auto it = bhDigis->begin(); it != bhDigis->end(); ++it) {
	//worker_->run1(evt, it, *heUncalibRechits);

	//KH HGCHEDetId detId     = (it->id());
	//KH int        layer     = detId.layer();
	DetId      detId     = it->id();
	int        layer     = ((geomType == 0) ? HGCalDetId(detId).layer() :
				  ((geomType == 1) ? HGCSiliconDetId(detId).layer() :
				   HGCScintillatorDetId(detId).layer()));
	HGCSample  hgcSample = it->sample(SampleIndx);
	uint16_t   toa       = hgcSample.toa();
	uint16_t   adc       = hgcSample.data();
	fill(detId, geom0, index, layer, adc, toa);
	  
	std::vector<float> digis;
	std::vector<int> ind;
	uint16_t adcSum = 0;
	for (int i=0; i< it->size(); i++){
	  HGCSample  hgcSampleTmp = it->sample(i);
	  digis.push_back(hgcSampleTmp.data());
	  ind.push_back(i);
	  
	  adcSum += hgcSampleTmp.data();

	  if (debug){
	    printf("HE isample: %6d, adc: %8d (%8d)\n",i,hgcSampleTmp.data(),adc);
	  }
	}
	v_v_samples     -> push_back ( digis );
	v_v_sampleIndex -> push_back ( ind );
	v_adcSum    -> push_back ( adcSum );
      }
    }
    
    //-----------------------------------------------------
    // Put things into the event
    //-----------------------------------------------------
    
    dumpAlgo(iEvent);

  }

 public:
  
 HGCalTupleMaker_HGCDigis(const edm::ParameterSet& iConfig) :
    m_HGCDigisTags (iConfig.getUntrackedParameter<std::vector<edm::InputTag> >("source")),
    m_geometrySource (iConfig.getUntrackedParameter<std::vector<std::string> >("geometrySource")),
    m_prefix         (iConfig.getUntrackedParameter<std::string>  ("Prefix")),
    m_suffix         (iConfig.getUntrackedParameter<std::string>  ("Suffix")),
    SampleIndx       (iConfig.getUntrackedParameter<int>("SampleIndx",2)) {

    m_HGCEEDigisToken = consumes<HGCalDigiCollection>(m_HGCDigisTags[0]);
    m_HGCHEDigisToken = consumes<HGCalDigiCollection>(m_HGCDigisTags[1]);
    m_HGCBHDigisToken = consumes<HGCalDigiCollection>(m_HGCDigisTags[2]);

    m_HGCDigisTokens.push_back(m_HGCEEDigisToken);
    m_HGCDigisTokens.push_back(m_HGCHEDigisToken);
    m_HGCDigisTokens.push_back(m_HGCBHDigisToken);
    
    produces<std::vector<float> > ( m_prefix + "Eta"    + m_suffix );
    produces<std::vector<float> > ( m_prefix + "Phi"    + m_suffix );
    if (detid_store){
      produces<std::vector<int> >   ( m_prefix + "IEta"   + m_suffix );
      produces<std::vector<int> >   ( m_prefix + "IPhi"   + m_suffix );
      produces<std::vector<int> >   ( m_prefix + "CellU"  + m_suffix );
      produces<std::vector<int> >   ( m_prefix + "CellV"  + m_suffix );
      produces<std::vector<int> >   ( m_prefix + "WaferU" + m_suffix );
      produces<std::vector<int> >   ( m_prefix + "WaferV" + m_suffix );
    }
    produces<std::vector<int>   > ( m_prefix + "Layer"  + m_suffix );
    produces<std::vector<uint16_t > > ( m_prefix + "ADC" + m_suffix );
    produces<std::vector<uint16_t > > ( m_prefix + "ADCSum" + m_suffix );
    produces<std::vector<float> > ( m_prefix + "Toa" + m_suffix );
    produces<std::vector<float> > ( m_prefix + "Posx"   + m_suffix );
    produces<std::vector<float> > ( m_prefix + "Posy"   + m_suffix );
    produces<std::vector<float> > ( m_prefix + "Posz"   + m_suffix );
    produces<std::vector<int>   > ( m_prefix + "Index"  + m_suffix );

    produces<std::vector<std::vector<float> > > ( m_prefix + "Samples"  + m_suffix );
    produces<std::vector<std::vector<int> > >   ( m_prefix + "SampleIndex"  + m_suffix );

    //produces<std::vector<int>   > ( m_prefix + "Flags"  + m_suffix );
    //produces<std::vector<int>   > ( m_prefix + "Aux"    + m_suffix );
    //produces<std::vector<float> > ( m_prefix + "Time"   + m_suffix );
    }
  
  std::unique_ptr<std::vector<float> > v_eta;
  std::unique_ptr<std::vector<float> > v_phi;
  std::unique_ptr<std::vector<int  > > v_layer;
  std::unique_ptr<std::vector<uint16_t > > v_adc;
  std::unique_ptr<std::vector<uint16_t > > v_adcSum;
  std::unique_ptr<std::vector<float> > v_toa;
  std::unique_ptr<std::vector<float> > v_posx;
  std::unique_ptr<std::vector<float> > v_posy;
  std::unique_ptr<std::vector<float> > v_posz;
  std::unique_ptr<std::vector<int>   > v_index;
  std::unique_ptr<std::vector<std::vector<float> > > v_v_samples;
  std::unique_ptr<std::vector<std::vector<int> > >   v_v_sampleIndex;

  std::unique_ptr<std::vector<int> > v_ieta;
  std::unique_ptr<std::vector<int> > v_iphi;
  std::unique_ptr<std::vector<int> > v_cellu;
  std::unique_ptr<std::vector<int> > v_cellv;
  std::unique_ptr<std::vector<int> > v_waferu;
  std::unique_ptr<std::vector<int> > v_waferv;
  
  void beginRun(const edm::Run&, const edm::EventSetup& iSetup){

    //initiating HGC Geometry
    for (size_t i=0; i<m_geometrySource.size(); i++) {
      
      // HCAL for BH/HEB
      if (m_geometrySource[i].find("HCal") != std::string::npos) {
        edm::ESHandle<HcalDDDSimConstants> pHSNDC;
        iSetup.get<HcalSimNumberingRecord>().get(pHSNDC);
        if (pHSNDC.isValid()) {
          hcCons_ = pHSNDC.product();
          hgcCons_.push_back(0);
        } else {
          edm::LogWarning("HGCalValid") << "Cannot initiate HcalDDDSimConstants: "
                                        << m_geometrySource[i] << std::endl;
        }
        edm::ESHandle<HcalDDDRecConstants> pHRNDC;
        iSetup.get<HcalRecNumberingRecord>().get(pHRNDC);
        if (pHRNDC.isValid()) {
          hcConr_ = pHRNDC.product();
        } else {
          edm::LogWarning("HGCalValid") << "Cannot initiate HcalDDDRecConstants: "
                                        << m_geometrySource[i] << std::endl;
        }
        edm::ESHandle<CaloGeometry> caloG;
        iSetup.get<CaloGeometryRecord>().get(caloG);
        if (caloG.isValid()) {
          const CaloGeometry* geo = caloG.product();
          hcGeometry_ = geo->getSubdetectorGeometry(DetId::Hcal,HcalBarrel);
          hgcGeometry_.push_back(0);
        } else {
          edm::LogWarning("HGCalValid") << "Cannot initiate HcalGeometry for "
                                        << m_geometrySource[i] << std::endl;
        }
      }
      // HGC for EE & HEF
      else {
        edm::ESHandle<HGCalDDDConstants> hgcCons;
        iSetup.get<IdealGeometryRecord>().get(m_geometrySource[i],hgcCons);
        if (hgcCons.isValid()) {
          hgcCons_.push_back(hgcCons.product());
        } else {
          edm::LogWarning("HGCalValid") << "Cannot initiate HGCalDDDConstants for "
                                        << m_geometrySource[i] << std::endl;
        }
        edm::ESHandle<HGCalGeometry> hgcGeom;
        iSetup.get<IdealGeometryRecord>().get(m_geometrySource[i],hgcGeom);     
        if(hgcGeom.isValid()) {
          hgcGeometry_.push_back(hgcGeom.product());    
        } else {
          edm::LogWarning("HGCalValid") << "Cannot initiate HGCalGeometry for "
                                        << m_geometrySource[i] << std::endl;
        }
      }
    } // loop over geometry source
  } // beginRun


 protected:

  void loadAlgo(){
    v_eta    = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());
    v_phi    = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());
    if (detid_store){
    v_ieta   = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    v_iphi   = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    v_cellu  = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    v_cellv  = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    v_waferu = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    v_waferv = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    }
    v_layer  = std::unique_ptr<std::vector<int  > > ( new std::vector<int  > ());
    v_adc    = std::unique_ptr<std::vector<uint16_t > > ( new std::vector<uint16_t > ());
    v_adcSum = std::unique_ptr<std::vector<uint16_t > > ( new std::vector<uint16_t > ());
    v_toa    = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());
    v_posx   = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());
    v_posy   = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());
    v_posz   = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());
    v_index = std::unique_ptr<std::vector<int>   > ( new std::vector<int  > ());

    v_v_samples     = std::unique_ptr<std::vector<std::vector<float> > > ( new std::vector<std::vector<float> > ());
    v_v_sampleIndex = std::unique_ptr<std::vector<std::vector<int> > >   ( new std::vector<std::vector<int> > ());

  }
  
  void dumpAlgo( edm::Event & iEvent ){
    iEvent.put( move(v_eta    ), m_prefix + "Eta"    + m_suffix );
    iEvent.put( move(v_phi    ), m_prefix + "Phi"    + m_suffix );
    if (detid_store){
    iEvent.put( move(v_ieta   ), m_prefix + "IEta"   + m_suffix );
    iEvent.put( move(v_iphi   ), m_prefix + "IPhi"   + m_suffix );
    iEvent.put( move(v_cellu  ), m_prefix + "CellU"  + m_suffix );
    iEvent.put( move(v_cellv  ), m_prefix + "CellV"  + m_suffix );
    iEvent.put( move(v_waferu ), m_prefix + "WaferU" + m_suffix );
    iEvent.put( move(v_waferv ), m_prefix + "WaferV" + m_suffix );
    }
    iEvent.put( move(v_layer  ), m_prefix + "Layer"  + m_suffix );
    iEvent.put( move(v_adc    ), m_prefix + "ADC"    + m_suffix );
    iEvent.put( move(v_adcSum ), m_prefix + "ADCSum" + m_suffix );
    iEvent.put( move(v_toa    ), m_prefix + "Toa"    + m_suffix );
    iEvent.put( move(v_posx   ), m_prefix + "Posx"   + m_suffix );
    iEvent.put( move(v_posy   ), m_prefix + "Posy"   + m_suffix );
    iEvent.put( move(v_posz   ), m_prefix + "Posz"   + m_suffix );
    iEvent.put( move(v_index  ), m_prefix + "Index" + m_suffix );

    iEvent.put( move(v_v_samples ),     m_prefix + "Samples"     + m_suffix );
    iEvent.put( move(v_v_sampleIndex ), m_prefix + "SampleIndex" + m_suffix );
  }

  //template<class Digi>          void HcalDigisValidation::reco(const edm::Event& iEvent, const edm::EventSetup& iSetup, const edm::EDGetTokenT<edm::SortedCollection<Digi> > & tok)
  //template<class dataFrameType> void HcalDigisValidation::reco(const edm::Event& iEvent, const edm::EventSetup& iSetup, const edm::EDGetTokenT<HcalDataFrameContainer<dataFrameType> > & tok)  
 
  template<class T1, class T2>
  void fill(const T1& detId, const T2* geom, 
	    int index, int layer, uint16_t adc, double toa) {

    //KH--- 
    double rout_layer[52]={
      1567.5, 1567.5, 1575.6, 1575.6, 1583.7,
      1583.7, 1591.8, 1591.8, 1599.9, 1599.9,
      1608.0, 1608.0, 1616.1, 1616.1, 1624.2,
      1624.2, 1632.2, 1632.2, 1640.4, 1640.4,
      1648.5, 1648.5, 1656.6, 1656.6, 1664.7,
      1664.7, 1672.8, 1672.8, 1696.9, 1713.5,
      1730.1, 1746.7, 1763.3, 1779.9, 1796.4,
      1844.2, 1907.6, 1971.0, 2034.5, 2097.9,
      2184.6, 2299.8, 2415.0, 2530.2, 2645.3,
      2664.0, 2664.0, 2664.0, 2664.0, 2664.0,
      2664.0, 2664.0
    };
    int layer_index=layer-1;
    if (index!=0) layer_index=layer+27;
    //KH---
    
    GlobalPoint global = geom->getPosition(detId);
    
    if (debug) std::cout << layer << " " << global.z() << std::endl;

    //KH----
    bool print_debug_geom=false;
    if (debug_geom){
      if ( fabs(global.eta()) > 3.01 || global.perp()>rout_layer[layer_index]/10.+5.) {
	std::cout << "digi eta>3.01 or r>rmax+5cm" << std::endl;
	print_debug_geom=true;
      }
    }

    HGCalGeometryMode::GeometryMode mode = geom->topology().geomMode();

    int    cellU=-100, cellV=-100, waferU=-100, waferV=-100;
    int    ieta=-100, iphi=-100, ietaAbs=-100;

    if ((mode == HGCalGeometryMode::Hexagon8) ||
	(mode == HGCalGeometryMode::Hexagon8Full)){
      HGCSiliconDetId detId0 = HGCSiliconDetId(detId);
      cellU            = detId0.cellU();
      cellV            = detId0.cellV();
      waferU           = detId0.waferU();
      waferV           = detId0.waferV();
      if (print_debug_geom){
	printf("Digis(Hexagon8) [cellU/V, waferU/V, eta, phil, layer, index]: %4d %4d %4d %4d %6.2f %6.2f %4d %4d\n",
	       cellU,cellV,waferU,waferV,global.eta(),double(global.phi()),layer,index);
	std::cout << detId0  << std::endl;
      }
    }
    else if (mode == HGCalGeometryMode::Trapezoid){
      HGCScintillatorDetId detId0 = HGCScintillatorDetId(detId);
      ietaAbs          = detId0.ietaAbs();
      ieta             =ietaAbs;
      if (detId0.zside()<0) ieta=-ietaAbs;
      iphi             = detId0.iphi();
      if (print_debug_geom){
	printf("Digis(Trapezoid) [ieta, iphi, eta, phi, r(rmax), layer, index]: %4d %4d %6.2f %6.2f %6.2f ( %6.2f ) %4d %4d\n",
	       ieta,iphi,global.eta(),double(global.phi()),global.perp(),rout_layer[layer_index]/10.,layer,index);
	std::cout << detId0  << std::endl;
      }
    }
    //KH---
    
    v_eta    -> push_back ( global.eta() );
    v_phi    -> push_back ( global.phi() );
    if (detid_store){
    v_ieta   -> push_back ( ieta         );
    v_iphi   -> push_back ( iphi         );
    v_cellu  -> push_back ( cellU        );
    v_cellv  -> push_back ( cellV        );
    v_waferu -> push_back ( waferU       );
    v_waferv -> push_back ( waferV       );
    }
    v_layer  -> push_back ( layer        );
    v_adc    -> push_back ( adc          );
    v_toa    -> push_back ( toa       );
    v_posx   -> push_back ( global.x()   );
    v_posy   -> push_back ( global.y()   );
    v_posz   -> push_back ( global.z()   );
    v_index  -> push_back ( index        );
  }
    
};

#endif
