#ifndef HGCalTupleMaker_HGCSimHits_h
#define HGCalTupleMaker_HGCSimHits_h

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ESTransientHandle.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/transform.h"

#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ESTransientHandle.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/transform.h"

// PCaloHits objects
#include "SimDataFormats/CaloHit/interface/PCaloHitContainer.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"

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

//
// Class definition
// 
class HGCalTupleMaker_HGCSimHits : public edm::EDProducer {
 protected:

  std::vector<edm::InputTag> m_PCaloHitsTags;
  std::vector<edm::EDGetTokenT<edm::PCaloHitContainer>> m_PCaloHitsTokens;

  std::vector<std::string> m_geometrySource;

  const std::string     m_prefix;
  const std::string     m_suffix;

  bool debug=false;
  bool debug_geom=false;
  bool detid_store=true;
  
  //HGC Geometry                                                                                                     
  std::vector<const HGCalDDDConstants*> hgcCons_;
  std::vector<const HGCalGeometry*>     hgcGeometry_;
  const HcalDDDSimConstants*            hcCons_;
  const HcalDDDRecConstants*            hcConr_;
  const CaloSubdetectorGeometry*        hcGeometry_;

  edm::ESHandle<CaloGeometry> geometry;
  
  std::map<uint32_t, HepGeom::Transform3D> transMap_;
    
  void produce( edm::Event & iEvent, const edm::EventSetup & iSetup ) { 

    if (debug) std::cout << "HGCalTupleMaker_HGCSimHits: produce starts" << std::endl;
    
    //-----------------------------------------------------
    // Prepare to put things into event
    //-----------------------------------------------------
    
    loadAlgo();
    std::map<int,float> m_energy;
    std::map<int,int> m_ieta;
    std::map<int,int> m_iphi;
    std::map<int,int> m_layer;
    std::map<int,int> m_index;


    //-----------------------------------------------------
    // edm::Handles
    //-----------------------------------------------------

    edm::Handle<edm::PCaloHitContainer> PCaloHits;

    //
    // Loop over PCaloHit containers
    //
    for( typename std::vector<edm::EDGetTokenT<edm::PCaloHitContainer> >::const_iterator
	   token = m_PCaloHitsTokens.begin(); token != m_PCaloHitsTokens.end(); ++token ) {
      unsigned index(token - m_PCaloHitsTokens.begin());
      PCaloHits.clear();

      //
      if (debug) std::cout << "HGCalTupleMaker_HGCSimHits: " << hgcCons_[index]->geomMode() << " " << index << std::endl;
      if (debug) std::cout << " Hexagon8: "     << HGCalGeometryMode::Hexagon8
		<< " Hexagon8Full: " << HGCalGeometryMode::Hexagon8Full
		<< " Hexagon: "      << HGCalGeometryMode::Hexagon
		<< " HexagonFull: "  << HGCalGeometryMode::HexagonFull
		<< " Square: "       << HGCalGeometryMode::Square
		<< " Trapezoid: "    << HGCalGeometryMode::Trapezoid
		<< std::endl;

      //
      // Geometry & looping over rechits
      //                                                                                                              
      std::string nameDetector_ = m_geometrySource[index];
      if (debug) std::cout << nameDetector_ << std::endl;

      //
      // getByToken
      // 
      iEvent.getByToken(*token, PCaloHits);
      if( PCaloHits.isValid() && !PCaloHits->empty() ) {
	if (debug) std::cout << "Input found" << std::endl;
	edm::LogInfo("Input found") << m_PCaloHitsTags.at(index);
      } else {
	if (debug) std::cout << "Input not found" << std::endl;
	edm::LogInfo("Input not found") << m_PCaloHitsTags.at(index);
	continue;
      }
      if (debug) std::cout << nameDetector_ << " " << PCaloHits->size() << std::endl;

      //
      // Loop over PCaloHits
      //
      for (const auto & it : *(PCaloHits.product())) {	  

	int    cell, type, sector, subsector, layer, zside;
	int    cellU=-100, cellV=-100, waferU=-100, waferV=-100;
	int    ieta=-100, iphi=-100, ietaAbs=-100;
	
	int    subdet(0);
	HepGeom::Point3D<float> gcoord;
	
	unsigned int id_ = it.id();

	// 
	if (nameDetector_ == "HCal") {

	  //
	  // HcalHitRelabeller method:
	  // - Validation/HGCalValidation/plugins/HGCalSimHitValidation.cc
	  //
	  HcalDetId detId = HcalHitRelabeller::relabel(id_,hcConr_);
	  subdet           = detId.subdet();

	  if (debug) std::cout << it.energy() << " " << subdet << std::endl;

	  if (debug && it.energy()>0.5) std::cout << "HGCalTupleMaker_HGCSimHits: " 
					 << it.energy() << " " 
					 << nameDetector_ << " " 
					 << subdet << " " << cell << " " << sector << std::endl;

	  //
	  // Use CaloCellGeometry getPosition
	  // 
	  //const CaloCellGeometry* cellGeometry = hcGeometry_->getGeometry(detId);
	  ////hcGeometry_ = geo->getSubdetectorGeometry(DetId::Hcal,HcalBarrel);
	  ////double etaS = cellGeometry->getPosition().eta();
	  ////double phiS = cellGeometry->getPosition().phi();

	  iSetup.get<CaloGeometryRecord>().get (geometry);
	  auto cellGeometry = geometry->getSubdetectorGeometry(detId)->getGeometry(detId);	  
	  
	  //
	  // Use CaloCellGeometry getPosition() method at the end
	  // 
	  gcoord = HepGeom::Point3D<float>(cellGeometry->getPosition().x(),
					   cellGeometry->getPosition().y(),
					   cellGeometry->getPosition().z());

	  if (debug) 
	  std::cout << "HCAL geom comparison: "
	    /*
		    << "(" << xp         << ", " << yp         << ", " << zp         << ") "  
		    << rho << " "
	    */
		    << "(" << gcoord.x() << ", " << gcoord.y() << ", " << gcoord.z() << ") "  
		    << "(" << cellGeometry->getPosition().x() << ", " << cellGeometry->getPosition().y() << ", " << cellGeometry->getPosition().z() << ") "  
		    << std::endl;

	} else {

	  //
	  // HGCAL geometry, not relying on HCAL
	  //
	  
	  if (debug && it.energy()>0.5) std::cout << "HGCalTupleMaker_HGCSimHits: " 
					 << it.energy() << " " 
					 << nameDetector_ << " " 
					 << subdet << " " << cell << " " << sector << std::endl;
	  
	  if (debug) std::cout << "HGCalTupleMaker_HGCSimHits: " << hgcCons_[index]->geomMode() << std::endl;

	  //
	  // Square
	  //
	  if (hgcCons_[index]->geomMode() == HGCalGeometryMode::Square) {

	    if (debug) std::cout << "HGCalTupleMaker_HGCSimHits: in the square mode." << std::endl;

	    HGCalTestNumbering::unpackSquareIndex(id_, zside, layer, sector, subsector, cell);
	    std::pair<float,float> xy = hgcCons_[index]->locateCell(cell,layer,subsector,false);
	    const HepGeom::Point3D<float> lcoord(xy.first,xy.second,0);
	    bool symmDet_=true;
	    int subs = (symmDet_ ? 0 : subsector);
	    id_      = HGCalTestNumbering::packSquareIndex(zside,layer,sector,subs,0);
	    gcoord   = (transMap_[id_]*lcoord); // 

	  } else {

	    if (debug) std::cout << "HGCalTupleMaker_HGCSimHits: in the non-square mode." << std::endl;

	    std::pair<float,float> xy;

	    //
	    // HF nose
	    //
	    if (m_geometrySource[index]=="HGCalHFNoseSensitive") {
	      HFNoseDetId detId = HFNoseDetId(id_);
	      subdet   = detId.subdetId();
	      cellU    = detId.cellU();
	      cellV    = detId.cellV();
	      waferU   = detId.waferU();
	      waferV   = detId.waferV();
	      type     = detId.type();
	      layer    = detId.layer();
	      zside    = detId.zside();
	      xy       = hgcCons_[index]->locateCell(layer,waferU,waferV,cellU,cellV,
						 false,true);	    
	    //
	    // Hexagons
	    //
	    } else if
	      ((hgcCons_[index]->geomMode() == HGCalGeometryMode::Hexagon8) ||
	       (hgcCons_[index]->geomMode() == HGCalGeometryMode::Hexagon8Full)) {

	      HGCSiliconDetId detId = HGCSiliconDetId(id_);
	      subdet           = ForwardEmpty;
	      cellU            = detId.cellU();
	      cellV            = detId.cellV();
	      waferU           = detId.waferU();
	      waferV           = detId.waferV();
	      type             = detId.type();
	      layer            = detId.layer();
	      zside            = detId.zside();
	      
	      xy = hgcCons_[index]->locateCell(layer,waferU,waferV,cellU,cellV,false,true);

	    //
	    // Trapezoid
	    //
	    } else if (hgcCons_[index]->geomMode() == HGCalGeometryMode::Trapezoid) {

	      HGCScintillatorDetId detId = HGCScintillatorDetId(id_);
	      subdet           = ForwardEmpty;
	      ietaAbs          = detId.ietaAbs();
	      iphi             = detId.iphi();
	      subsector        = 1;
	      type             = detId.type();
	      layer            = detId.layer();
	      zside            = detId.zside();

	      xy = hgcCons_[index]->locateCellTrap(layer,ietaAbs,iphi,false);

	      ieta=ietaAbs;
	      if (zside<0) ieta=-ietaAbs;
	      
	      if (debug){
	      double zp = hgcCons_[index]->waferZ(layer,false)/10.; // mm->cm
	      double xp = (zp<0) ? -xy.first/10 : xy.first/10; //mm
	      double yp = xy.second/10; //mm	  
	      std::cout << "HGC geom comparison: "
				   << "(" << xp         << ", " << yp         << ", " << zp         << ") "  
				   << std::endl;
	      }
	      
	    //
	    // Others
	    //
	    } else {

	      HGCalTestNumbering::unpackHexagonIndex(id_, subdet, zside, layer, sector, type, cell);
	      xy = hgcCons_[index]->locateCell(cell,layer,sector,false);

	    }

	    // 
	    // Flip x for negative z
	    //
	    double zp = hgcCons_[index]->waferZ(layer,false)/10.; // mm->cm
	    if (zside < 0) zp = -zp;
	    float  xp = (zp < 0) ? -xy.first/10 : xy.first/10; // mm->cm
	    float  yp = xy.second/10; //mm->cm
	    gcoord = HepGeom::Point3D<float>(xp,yp,zp); // 
	    
	  }

	}  // if nameDetector_ 

	double tof = (gcoord.mag()*CLHEP::mm)/CLHEP::c_light; 
	
	v_energy -> push_back ( it.energy() );

	if(m_energy.find(id_) == m_energy.end())
	  {
	    m_energy[id_] = it.energy();
	    m_ieta[id_] = ieta;
	    m_iphi[id_] = iphi;
	    m_layer[id_] = layer;
	    m_index[id_] = index;
	  }
	else
	  {
	    m_energy[id_] += it.energy();
	  }
	

	//KH--- 
	if (debug_geom){
	  
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

	  if ( fabs(gcoord.getEta()) > 3. || gcoord.perp()>rout_layer[layer_index]/10.+5.) {
	    if ((hgcCons_[index]->geomMode() == HGCalGeometryMode::Hexagon8) ||
		(hgcCons_[index]->geomMode() == HGCalGeometryMode::Hexagon8Full)){
	      printf("Simhits(Hexagon8) [cellU/V, waferU/V, eta, phil, layer, index, detid]: %4d %4d %4d %4d %6.2f %6.2f %4d %4d ",
		     cellU,cellV,waferU,waferV,gcoord.getEta(),gcoord.getPhi(),layer,index);
	      std::cout << id_  << std::endl;
	      HGCSiliconDetId detId = HGCSiliconDetId(id_);
	      std::cout << detId << std::endl;
	    }
	    else if (hgcCons_[index]->geomMode() == HGCalGeometryMode::Trapezoid){
	      printf("Simhits(Trapezoid) [ieta, iphi, eta, phi, layer, index, detid]: %4d %4d %6.2f %6.2f %4d %4d ",
		     ieta,iphi,gcoord.getEta(),gcoord.getPhi(),layer,index);
	      std::cout << id_  << std::endl;
	      HGCScintillatorDetId detId = HGCScintillatorDetId(id_);
	      std::cout << detId << std::endl;
	    }
	  }
	
	} // if debug_geom
	//KH---
	
	if (debug) std::cout << "HGCalTupleMaker_HGCSimHits: " << nameDetector_ << " "
		  << it.time() << " "
		  << it.time()-tof << " "
		  << subdet << " "
		  << layer << " "
		  << "(" << gcoord.x() << ", " << gcoord.y() << ", " << gcoord.z() << ") "  
		  << gcoord.getEta() << " "
		  << gcoord.getPhi() << " "
		  << std::endl;

	v_time   -> push_back ( it.time() );	
	//v_time   -> push_back ( it.time()-tof );	
	v_subdet -> push_back ( subdet );
	v_layer  -> push_back ( layer );
	v_index  -> push_back ( index );
	v_eta    -> push_back ( gcoord.getEta() );
	v_phi    -> push_back ( gcoord.getPhi() );
	if (detid_store){
	  v_ieta   -> push_back ( ieta         );
	  v_iphi   -> push_back ( iphi         );
	  v_cellu  -> push_back ( cellU        );
	  v_cellv  -> push_back ( cellV        );
	  v_waferu -> push_back ( waferU       );
	  v_waferv -> push_back ( waferV       );
	}
	v_posx   -> push_back ( gcoord.x() );
	v_posy   -> push_back ( gcoord.y() );
	v_posz   -> push_back ( gcoord.z() );
	
      } // for-loop of PCaloHits
    } // Looping over different PCaloHit collections

    // from map to vector
    for(auto itr = m_energy.begin(); itr != m_energy.end(); ++itr)
      {
	v_int_energy -> push_back(m_energy[itr->first]);
	v_int_ieta   -> push_back(m_ieta[itr->first]);
	v_int_iphi   -> push_back(m_iphi[itr->first]);
	v_int_layer  -> push_back(m_layer[itr->first]);
	v_int_index  -> push_back(m_index[itr->first]);
      }

    //-----------------------------------------------------
    // Put things into the event
    //-----------------------------------------------------
    dumpAlgo(iEvent);

  }
  
 public:
  
 HGCalTupleMaker_HGCSimHits(const edm::ParameterSet& iConfig) :
  m_PCaloHitsTags (iConfig.getUntrackedParameter<std::vector<edm::InputTag> >("source")),
    m_PCaloHitsTokens(edm::vector_transform(m_PCaloHitsTags, [this](edm::InputTag const & tag){
	  return consumes<edm::PCaloHitContainer>(tag);})),
    m_geometrySource (iConfig.getUntrackedParameter<std::vector<std::string> >("geometrySource")),
    m_prefix         (iConfig.getUntrackedParameter<std::string>  ("Prefix")),
    m_suffix         (iConfig.getUntrackedParameter<std::string>  ("Suffix")) {

    produces<std::vector<float> > ( m_prefix + "IntEnergy" + m_suffix );
    produces<std::vector<int> > ( m_prefix + "IntIEta" + m_suffix );
    produces<std::vector<int> > ( m_prefix + "IntIPhi" + m_suffix );
    produces<std::vector<int> > ( m_prefix + "IntLayer" + m_suffix );
    produces<std::vector<int> > ( m_prefix + "IntIndex" + m_suffix );

    produces<std::vector<float> > ( m_prefix + "Energy" + m_suffix );
    produces<std::vector<float> > ( m_prefix + "Time"   + m_suffix );
    produces<std::vector<int>   > ( m_prefix + "Subdet" + m_suffix );
    produces<std::vector<int>   > ( m_prefix + "Layer"  + m_suffix );
    produces<std::vector<int>   > ( m_prefix + "Index"  + m_suffix );
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
    produces<std::vector<float> > ( m_prefix + "Posx"   + m_suffix );
    produces<std::vector<float> > ( m_prefix + "Posy"   + m_suffix );
    produces<std::vector<float> > ( m_prefix + "Posz"   + m_suffix );

  }
  std::unique_ptr<std::vector<float> > v_int_energy;
  std::unique_ptr<std::vector<int> > v_int_ieta;
  std::unique_ptr<std::vector<int> > v_int_iphi;
  std::unique_ptr<std::vector<int> > v_int_layer;
  std::unique_ptr<std::vector<int> > v_int_index;

  std::unique_ptr<std::vector<float> > v_energy;
  std::unique_ptr<std::vector<float> > v_time;
  std::unique_ptr<std::vector<int>   > v_id;
  std::unique_ptr<std::vector<int>   > v_index; // index for different input collections
  std::unique_ptr<std::vector<int>   > v_subdet;
  
  std::unique_ptr<std::vector<int  > > v_cell; // 
  std::unique_ptr<std::vector<int  > > v_sector; // wafer
  std::unique_ptr<std::vector<int  > > v_subsector; // type
  std::unique_ptr<std::vector<int  > > v_layer;
  std::unique_ptr<std::vector<int  > > v_zside;

  std::unique_ptr<std::vector<float> > v_posx;
  std::unique_ptr<std::vector<float> > v_posy;
  std::unique_ptr<std::vector<float> > v_posz;
  std::unique_ptr<std::vector<float> > v_eta;
  std::unique_ptr<std::vector<float> > v_phi;

  std::unique_ptr<std::vector<int  > > v_ieta;
  std::unique_ptr<std::vector<int  > > v_iphi;
  std::unique_ptr<std::vector<int  > > v_cellu;
  std::unique_ptr<std::vector<int  > > v_cellv;
  std::unique_ptr<std::vector<int  > > v_waferu;
  std::unique_ptr<std::vector<int  > > v_waferv;
  
 private:

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
    }

  }

 protected:

  void loadAlgo(){
    v_int_energy = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());
    v_int_ieta   = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    v_int_iphi   = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    v_int_layer  = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    v_int_index  = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());

    v_energy = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());
    v_time   = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());
    v_subdet = std::unique_ptr<std::vector<int>   > ( new std::vector<int  > ());
    v_layer  = std::unique_ptr<std::vector<int  > > ( new std::vector<int  > ());
    v_index  = std::unique_ptr<std::vector<int  > > ( new std::vector<int  > ());
    v_eta    = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());
    v_phi    = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());
    //if (detid_store){
    v_ieta   = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    v_iphi   = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    v_cellu  = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    v_cellv  = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    v_waferu = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    v_waferv = std::unique_ptr<std::vector<int> > ( new std::vector<int> ());
    //}
    v_posx   = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());
    v_posy   = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());
    v_posz   = std::unique_ptr<std::vector<float> > ( new std::vector<float> ());

  }
  
  void dumpAlgo( edm::Event & iEvent ){
    iEvent.put( move(v_int_energy ), m_prefix + "IntEnergy" + m_suffix );
    iEvent.put( move(v_int_ieta   ), m_prefix + "IntIEta"   + m_suffix );
    iEvent.put( move(v_int_iphi   ), m_prefix + "IntIPhi"   + m_suffix );
    iEvent.put( move(v_int_layer  ), m_prefix + "IntLayer"  + m_suffix );
    iEvent.put( move(v_int_index  ), m_prefix + "IntIndex"  + m_suffix );

    iEvent.put( move(v_energy ), m_prefix + "Energy" + m_suffix );
    iEvent.put( move(v_time   ), m_prefix + "Time"   + m_suffix );
    iEvent.put( move(v_subdet ), m_prefix + "Subdet" + m_suffix );
    iEvent.put( move(v_layer  ), m_prefix + "Layer"  + m_suffix );
    iEvent.put( move(v_index  ), m_prefix + "Index"  + m_suffix );
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
    iEvent.put( move(v_posx   ), m_prefix + "Posx"   + m_suffix );
    iEvent.put( move(v_posy   ), m_prefix + "Posy"   + m_suffix );
    iEvent.put( move(v_posz   ), m_prefix + "Posz"   + m_suffix );

  }  
    
};

#endif
