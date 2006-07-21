#include "PhysicsTools/RecoAlgos/interface/TrackSelector.h"
#include "FWCore/Framework/interface/EventPrincipal.h" 
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackExtra.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include <vector>
#include <utility>
using namespace reco;
using namespace std;
using namespace edm;

TrackSelectorBase::TrackSelectorBase( const ParameterSet & cfg ) :
  src_( cfg.getParameter<std::string>( "src" ) ),
  filter_( cfg.getParameter<bool>( "filter" ) ) {
  string alias( cfg.getParameter<std::string>( "@module_label" ) );
  produces<TrackCollection>().setBranchAlias( alias + "Tracks" );
  produces<TrackExtraCollection>().setBranchAlias( alias + "TrackExtras" );
  produces<TrackingRecHitCollection>().setBranchAlias( alias + "RecHits" );
}
 
TrackSelectorBase::~TrackSelectorBase() {
}

bool TrackSelectorBase::select( const Track & ) const {
  return true;
}

bool TrackSelectorBase::filter( Event& evt, const EventSetup& ) {
  Handle<TrackCollection> tracks;
  evt.getByLabel( src_, tracks );

  auto_ptr<TrackCollection> selTracks( new TrackCollection );
  auto_ptr<TrackExtraCollection> selTrackExtras( new TrackExtraCollection );
  auto_ptr<TrackingRecHitCollection> selHits( new TrackingRecHitCollection );

  TrackingRecHitRefProd rHits = evt.getRefBeforePut<TrackingRecHitCollection>();
  TrackExtraRefProd rTrackExtras = evt.getRefBeforePut<TrackExtraCollection>();
  TrackRefProd rTracks = evt.getRefBeforePut<TrackCollection>();

  size_t idx = 0, hidx = 0;
  for( TrackCollection::const_iterator trk = tracks->begin(); trk != tracks->end(); ++ trk ) {
    if( select( * trk ) ) {
      selTracks->push_back( Track( *trk ) );
      selTracks->back().setExtra( TrackExtraRef( rTrackExtras, idx ++ ) );
      selTrackExtras->push_back( TrackExtra ( trk->outerPosition(), trk->outerMomentum(), trk->outerOk() ) );
      TrackExtra & tx = selTrackExtras->back();
      for( trackingRecHit_iterator hit = trk->recHitsBegin(); hit != trk->recHitsEnd(); ++ hit ) {
	selHits->push_back( (*hit)->clone() );
	tx.add( TrackingRecHitRef( rHits, hidx ++ ) );
      }
    }
  }

  evt.put( selTracks );
  evt.put( selTrackExtras );
  evt.put( selHits );
  return ( selTracks->size() > 0 || ! filter_ );
}
