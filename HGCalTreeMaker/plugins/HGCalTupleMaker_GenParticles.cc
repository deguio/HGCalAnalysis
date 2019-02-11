#include "HGCalAnalysis/HGCalTreeMaker/interface/HGCalTupleMaker_GenParticles.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/Framework/interface/ESHandle.h"

HGCalTupleMaker_GenParticles::HGCalTupleMaker_GenParticles(const edm::ParameterSet& iConfig):
  inputTag    (iConfig.getUntrackedParameter<edm::InputTag>("source")),
  prefix      (iConfig.getUntrackedParameter<std::string>  ("Prefix")),
  suffix      (iConfig.getUntrackedParameter<std::string>  ("Suffix"))
{

  produces< std::vector< double > >(prefix + "Pt"  + suffix );
  produces< std::vector< double > >(prefix + "P"  + suffix );
  produces< std::vector< double > >(prefix + "Eta" + suffix );
  produces< std::vector< double > >(prefix + "Phi" + suffix );
  produces< std::vector< double > >(prefix + "M"   + suffix );
  produces< std::vector< int > >(prefix + "PdgId" + suffix );
  produces< std::vector< int > >(prefix + "Status" + suffix );
  //produces< std::vector< int > >(prefix + "Parent" + suffix );
  //produces< std::vector< int > >(prefix + "ParentId" + suffix );

  genCollectionToken_ = consumes<edm::View<reco::GenParticle>>(inputTag);

}

void HGCalTupleMaker_GenParticles::produce(edm::Event& iEvent, const edm::EventSetup& iSetup){

  std::unique_ptr<std::vector<double> >            pt                ( new std::vector<double>           ());
  std::unique_ptr<std::vector<double> >            p                 ( new std::vector<double>           ());
  std::unique_ptr<std::vector<double> >            eta               ( new std::vector<double>           ());
  std::unique_ptr<std::vector<double> >            phi               ( new std::vector<double>           ());
  std::unique_ptr<std::vector<double> >            mass              ( new std::vector<double>           ());

  std::unique_ptr<std::vector<int   > >            pdgid             ( new std::vector<int>              ());
  std::unique_ptr<std::vector<int   > >            status            ( new std::vector<int>              ());
  //std::unique_ptr<std::vector<int   > >            parent            ( new std::vector<int>              ());
  //std::unique_ptr<std::vector<int   > >            parentid          ( new std::vector<int>              ());

  edm::Handle< edm::View<reco::GenParticle> > genPartCands;
  iEvent.getByToken(genCollectionToken_, genPartCands);

  for(edm::View<reco::GenParticle>::const_iterator iPart = genPartCands->begin(); iPart != genPartCands->end(); ++iPart)
    {

      pt->push_back(iPart->pt());
      p->push_back(iPart->p());
      eta->push_back(iPart->eta());
      phi->push_back(iPart->phi());
      mass->push_back(iPart->mass());

      pdgid->push_back(iPart->pdgId());
      status->push_back(iPart->status());

    }

  iEvent.put(move( pt              ) , prefix + "Pt"            + suffix );
  iEvent.put(move( p               ) , prefix + "P"             + suffix );
  iEvent.put(move( eta             ) , prefix + "Eta"           + suffix );
  iEvent.put(move( phi             ) , prefix + "Phi"           + suffix );
  iEvent.put(move( mass            ) , prefix + "M"             + suffix );

  iEvent.put(move( pdgid           ) , prefix + "PdgId"         + suffix );
  iEvent.put(move( status          ) , prefix + "Status"        + suffix );
  //iEvent.put(move( parent          ) , prefix + "Parent"        + suffix );
  //iEvent.put(move( parentid        ) , prefix + "ParentId"      + suffix );

}
