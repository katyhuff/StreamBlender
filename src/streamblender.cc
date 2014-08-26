#include "streamblender.h"

namespace streamblender {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StreamBlender::StreamBlender(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {
      prefs_=Prefs_();
      cyclus::Warn<cyclus::EXPERIMENTAL_WARNING>("the StreamBlender is experimental.");
    };

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// pragmas

#pragma cyclus def schema streamblender::StreamBlender

#pragma cyclus def annotations streamblender::StreamBlender

#pragma cyclus def initinv streamblender::StreamBlender

#pragma cyclus def snapshotinv streamblender::StreamBlender

#pragma cyclus def infiletodb streamblender::StreamBlender

#pragma cyclus def snapshot streamblender::StreamBlender

#pragma cyclus def clone streamblender::StreamBlender

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamBlender::InitFrom(StreamBlender* m) {

  #pragma cyclus impl initfromcopy streamblender::StreamBlender

  prefs_=Prefs_();
  cyclus::toolkit::CommodityProducer::Copy(m);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamBlender::InitFrom(cyclus::QueryableBackend* b){

  #pragma cyclus impl initfromdb streamblender::StreamBlender

  prefs_=Prefs_();
  using cyclus::toolkit::Commodity;
  Commodity commod = Commodity(out_commod);
  cyclus::toolkit::CommodityProducer::Add(commod);
  cyclus::toolkit::CommodityProducer::SetCapacity(commod, capacity);
  cyclus::toolkit::CommodityProducer::SetCost(commod, cost);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamBlender::EnterNotify() {
  Facility::EnterNotify();

  using cyclus::toolkit::Commodity;
  Commodity commod = Commodity(out_commod);
  cyclus::toolkit::CommodityProducer::Add(commod);
  cyclus::toolkit::CommodityProducer::SetCapacity(commod, capacity);
  cyclus::toolkit::CommodityProducer::SetCost(commod, cost);
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StreamBlender::str() {

  std::string commods = "";
  for (std::vector<std::string>::iterator commod = in_commods.begin();
       commod != in_commods.end();
       commod++) {
    commods += (commod == in_commods.begin() ? "{" : ", ");
    commods += (*commod);
  }

  std::stringstream ss;
  ss << cyclus::Facility::str();
  ss << " has facility parameters {" << "\n"
     << "     Input Commodities = " << commods << ",\n"
     << "     Output Commodity = " << out_commod_() << ",\n"
     << "     Process Time = " << process_time_() << ",\n"
     << "     Maximum Inventory Size = " << max_inv_size_() << ",\n"
     << "     Capacity = " << capacity_() << ",\n"
     << "'}";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamBlender::Tick() {
  LOG(cyclus::LEV_INFO3, "SBlend") << prototype() << " is ticking {";

  // @TODO uncomment and implement
  //if (context()->time() == lifetime_) {
  //  EndLife_();
  //}

  LOG(cyclus::LEV_INFO3, "SBlend") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamBlender::Tock() {
  LOG(cyclus::LEV_INFO3, "SBlend") << prototype() << " is tocking {";
  BeginProcessing_(); // place unprocessed rawbuffs into processing

  std::set<std::string>::const_iterator it;
  //for (it = crctx_.in_commods().begin(); it != crctx_.in_commods().end(); ++it) {
  BlendStreams_();
  //}

  LOG(cyclus::LEV_INFO3, "SBlend") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
StreamBlender::GetMatlRequests() {
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::RequestPortfolio;
  using cyclus::Request;

  std::set<std::string>::const_iterator it;
  std::set<RequestPortfolio<Material>::Ptr> ports;
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  double amt = current_capacity();
  Material::Ptr mat = cyclus::NewBlankMaterial(amt);

  if (amt > cyclus::eps()) {
    CapacityConstraint<Material> cc(amt);
    port->AddConstraint(cc);

    std::vector<std::string>::const_iterator it;
    std::vector<Request<Material>*> mutuals;
    for (it = sources.begin(); it != sources.end(); ++it) {
      mutuals.push_back(port->AddRequest(mat, this, *it)); 

      LOG(cyclus::LEV_INFO5, "SBlend") << prototype()
                                    << " requested "
                                    << mat->quantity()
                                    << " of " << *it;
    }
    port->AddMutualReqs(mutuals);
    ports.insert(port);
  } // if amt > eps

  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamBlender::AcceptMatlTrades(
  const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
  cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;

  std::map<std::string, Material::Ptr> mat_commods;

  std::vector< std::pair<cyclus::Trade<cyclus::Material>,
                         cyclus::Material::Ptr> >::const_iterator trade;

  // blob each material by commodity
  std::string commod;
  Material::Ptr mat;
  for (trade = responses.begin(); trade != responses.end(); ++trade) {
    commod = trade->first.request->commodity();
    mat = trade->second;
    AddMat_(commod, mat);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
StreamBlender::GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                          commod_requests) {
  using cyclus::BidPortfolio;
  using cyclus::Material;

  std::set<BidPortfolio<Material>::Ptr> ports;

  std::set<std::string>::const_iterator it;
  BidPortfolio<Material>::Ptr port = GetBids_(commod_requests,
                                              out_commod_(),
                                              &blendbuff);
  if (!port->bids().empty()) {
    ports.insert(port);
  }

  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamBlender::GetMatlTrades(
  const std::vector< cyclus::Trade<cyclus::Material> >& trades,
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
  cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::Trade;

  // for each trade, respond
  std::vector< Trade<Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    std::string commodity = it->request->commodity();
    double qty = std::min(it->amt, blendbuff.quantity());
    // create a material pointer representing what you can offer
    Material::Ptr response = TradeResponse_(qty, &blendbuff);

    responses.push_back(std::make_pair(*it, response));
    LOG(cyclus::LEV_INFO5, "SBlend") << prototype()
                                  << " just received an order"
                                  << " for " << it->amt
                                  << " of " << commodity
                                  << " and offered " << qty;

  }

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamBlender::AddMat_(std::string commod, 
    cyclus::Material::Ptr mat) {

  LOG(cyclus::LEV_INFO5, "SBlend") << prototype() << " is initially holding "
                                << rawbuffs_quantity() << " raw and "
                                << blendbuff.quantity() << " blended.";

  try {
    rawbuffs[commod].Push(mat);
    LOG(cyclus::LEV_INFO5, "SBlend") << prototype() << " added " << mat->quantity()
                                  << " of " << commod
                                  << " to its rawbuffs, which is holding "
                                  << rawbuffs_quantity() << " total.";
  } catch (cyclus::Error& e) {
    e.msg(Agent::InformErrorMsg(e.msg()));
    throw e;
  }


}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::BidPortfolio<cyclus::Material>::Ptr StreamBlender::GetBids_(
    cyclus::CommodMap<cyclus::Material>::type& commod_requests,
    std::string commod,
    cyclus::toolkit::ResourceBuff* buffer) {
  using cyclus::Bid;
  using cyclus::BidPortfolio;
  using cyclus::CapacityConstraint;
  using cyclus::Composition;
  using cyclus::Converter;
  using cyclus::Material;
  using cyclus::Request;
  using cyclus::ResCast;
  using cyclus::toolkit::ResourceBuff;

  BidPortfolio<Material>::Ptr port(new BidPortfolio<Material>());

  if (commod_requests.count(commod) > 0 && buffer->quantity() > 0) {
    std::vector<Request<Material>*>& requests = commod_requests.at(commod);

    // get offer composition
    Material::Ptr back = ResCast<Material>(buffer->Pop(ResourceBuff::BACK));
    Composition::Ptr comp = back->comp();
    buffer->Push(back);

    std::vector<Request<Material>*>::iterator it;
    for (it = requests.begin(); it != requests.end(); ++it) {
      Request<Material>* req = *it;
      double qty = std::min(req->target()->quantity(), buffer->quantity());
      Material::Ptr offer = Material::CreateUntracked(qty, comp);
      port->AddBid(req, offer, this);
    }

    CapacityConstraint<Material> cc(buffer->quantity());
    port->AddConstraint(cc);
  }

  return port;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr StreamBlender::TradeResponse_(
    double qty,
    cyclus::toolkit::ResourceBuff* buffer) {
  using cyclus::Material;
  using cyclus::ResCast;

  std::vector<Material::Ptr> manifest;
  try {
    // pop amount from buffer and blob it into one material
    manifest = ResCast<Material>(buffer->PopQty(qty));
  } catch(cyclus::Error& e) {
    e.msg(Agent::InformErrorMsg(e.msg()));
    throw e;
  }

  Material::Ptr response = manifest[0];
  crctx_.RemoveRsrc(response);
  for (int i = 1; i < manifest.size(); i++) {
    crctx_.RemoveRsrc(manifest[i]);
    response->Absorb(manifest[i]);
  }
  return response;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamBlender::BeginProcessing_(){
  std::map<std::string, cyclus::toolkit::ResourceBuff>::iterator it;
  for (it = rawbuffs.begin(); it != rawbuffs.end(); ++it){
    while (!(*it).second.empty()){
      try {
        processing[context()->time()][(*it).first].Push((*it).second.Pop());
        LOG(cyclus::LEV_DEBUG2, "SBlend") << "StreamBlender " << prototype()
                                        << " added resources to processing";
      } catch(cyclus::Error& e) {
        e.msg(Agent::InformErrorMsg(e.msg()));
        throw e;
      }
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::toolkit::ResourceBuff StreamBlender::MeetNeed_(int iso, int n){
  using cyclus::toolkit::ResourceBuff;

  LOG(cyclus::LEV_DEBUG2, "SBlend") << "StreamBlender " << prototype()
                                  << " is meeting a need for iso: " << iso;

  ResourceBuff iso_source_buff =  ResourceBuff();
  double need = n*GoalCompMap_()[iso];
  std::set<std::string>::const_iterator pref;
  std::set<std::string> preflist = prefs(iso/10000);
  for(pref = preflist.begin(); pref != preflist.end(); ++pref){
      double avail = processing[ready()][*pref].quantity();
      double diff = need - avail;
      double to_pop = std::min(need, avail);
      iso_source_buff.PushAll(processing[ready()][*pref].PopQty(to_pop));
      need = std::max(diff,0.0);
  }
  return iso_source_buff;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int StreamBlender::NPossible_(){

  bool first_run = true;
  int n_poss = 0;
  int prev = 0;
  std::map<int, double>::const_iterator it;
  cyclus::CompMap goal = GoalCompMap_();
  for(it = goal.begin(); it != goal.end(); ++it){
    int iso = it->first;
    double amt = it->second;
    double avail = AmtPossible_(processing[ready()],iso);
    int curr = int(std::floor(avail/amt));
    if(first_run){
      n_poss = curr;
      first_run = false;
    } else {
      prev = n_poss;
      n_poss = (prev < curr)? prev : curr;
    }
  }
  return n_poss;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double StreamBlender::AmtPossible_(std::map< std::string, 
    cyclus::toolkit::ResourceBuff> source_buffs, int iso) {
  using cyclus::toolkit::ResourceBuff;

  double avail = 0;

  std::map< std::string, ResourceBuff >::iterator it;
  for( it = source_buffs.begin(); it != source_buffs.end(); ++it){
    avail += AmtPossible_(&(it->second), iso);
  }
  return avail;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double StreamBlender::AmtPossible_(cyclus::toolkit::ResourceBuff* buff, int iso) {
  using cyclus::toolkit::MatQuery;
  using cyclus::toolkit::ResourceBuff;
  using cyclus::ResCast;
  using cyclus::Material;

  double avail = 0;
  int n = buff->count();
  for(int i = 0; i < n; ++i) {
    Material::Ptr front = ResCast<Material>(buff->Pop(ResourceBuff::FRONT));
    MatQuery mq = MatQuery(front);
    avail += mq.mass(iso);
    buff->Push(front);
  }
  return avail;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr StreamBlender::CollapseBuff(cyclus::toolkit::ResourceBuff* to_collapse){
  using cyclus::toolkit::Manifest;
  using cyclus::Material;
  using cyclus::ResCast;
  double qty =  to_collapse->quantity();
  Manifest manifest = to_collapse->PopQty(qty);


  Material::Ptr back = ResCast<Material>(manifest.back());
  manifest.pop_back();
  while ( !manifest.empty() ){
    back->Absorb(ResCast<Material>(manifest.back()));
    manifest.pop_back();
  }
  return back;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamBlender::Blend_(cyclus::toolkit::ResourceBuff* fabbed_fuel_buff, int n_poss){
  using cyclus::toolkit::Manifest;
  using cyclus::Material;
  using cyclus::ResCast;

  Material::Ptr soup = CollapseBuff(fabbed_fuel_buff);

  for( int i=0; i<n_poss; ++i){
    Material::Ptr goal_mat =  soup->ExtractComp(GoalCompMass_(), GoalComp_());
    blendbuff.Push(goal_mat);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Composition::Ptr StreamBlender::GoalComp_(){
  std::string out = out_recipe_();
  cyclus::Composition::Ptr recipe = context()->GetRecipe(out_recipe_());
  return recipe;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::CompMap StreamBlender::GoalCompMap_(){
  return GoalComp_()->mass();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double StreamBlender::GoalCompMass_(){
  double amt = 0;
  std::map<int, double>::const_iterator it;
  cyclus::CompMap goal = GoalCompMap_();
  for(it=goal.begin(); it!=goal.end(); ++it){
    amt += it->second;
  }
  return amt;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<std::string> StreamBlender::prefs(int iso){

  std::set<std::string> preflist;
  std::map<int, std::set<std::string> >::const_iterator it;
  it = prefs_.find(iso);
  if(it != prefs_.end()){
    preflist = it->second;
  } else {
    std::stringstream ss;
    ss << "Invalid pref. No source is named for iso: " << iso ;
    throw cyclus::ValueError(ss.str());
  }
  return preflist;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<int> StreamBlender::IsoIdx_(int iso){
  std::set<int> to_ret;
  int idx;

  std::vector<int>::iterator it = isos.begin();
  while(it != isos.end()){
    idx = find(it, isos.end(), iso) - isos.begin();
    if( idx < isos.size() ){
      to_ret.insert(idx);
    }
    ++it;
  }
  if( to_ret.size() == 0 ) {
    throw cyclus::KeyError("The iso was not found in the iso->source map");
  }
  return to_ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<std::string> StreamBlender::Sources_(int iso){
  std::set<std::string> to_ret;
  std::set<int> idxs = IsoIdx_(iso);
  std::set<int>::const_iterator idx;
  for(idx = idxs.begin(); idx !=idxs.end(); ++idx){
    to_ret.insert(sources[*idx]);
    LOG(cyclus::LEV_DEBUG3, "SBlend") << "StreamBlender " << prototype() 
                                     << " is matching iso: " << iso
                                     << " with source: " << sources[*idx];
  }
  return to_ret;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::map<int, std::set<std::string> > StreamBlender::Prefs_(){
  std::map<int, std::set<std::string> > to_ret;
  std::vector<int>::const_iterator iso;
  for( iso = isos.begin(); iso != isos.end(); ++iso) {
    if( to_ret.count(*iso) == 0 ){
      to_ret.insert(std::make_pair(*iso, Sources_(*iso)));
      LOG(cyclus::LEV_DEBUG3, "SBlend") << "StreamBlender " << prototype() 
                                     << " has matched iso: " << *iso
                                     << " with sources";
    }
  } 
  return to_ret;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamBlender::BlendStreams_(){
  using cyclus::Material;
  using cyclus::toolkit::ResourceBuff;

  int n = NPossible_();
  if( n > 0 ){
    std::map< int, std::set<std::string> >::const_iterator pref;
    ResourceBuff fabbed_fuel_buff;

    for(pref = prefs_.begin(); pref != prefs_.end(); ++pref){
      int iso = pref->first;
      ResourceBuff to_add_buff = MeetNeed_(iso, n);
      double qty = to_add_buff.quantity();
      fabbed_fuel_buff.PushAll(to_add_buff.PopQty(qty));
    }

    Blend_(&fabbed_fuel_buff, n);
    LOG(cyclus::LEV_DEBUG2, "SBlend") << "StreamBlender " << prototype() 
                                     << " is blending streams.";
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double StreamBlender::rawbuffs_quantity(std::string commod)const {
  using cyclus::toolkit::ResourceBuff;

  std::map<std::string, ResourceBuff>::const_iterator found;
  found = rawbuffs.find(commod);
  double amt;
  if ( found != rawbuffs.end() ){
    amt = (*found).second.quantity();
  } else {
    amt =0;
  }
  return amt;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double StreamBlender::rawbuffs_quantity() const {
  using cyclus::toolkit::ResourceBuff;

  double total = 0;
  std::map<std::string, ResourceBuff>::const_iterator it;
  for( it = rawbuffs.begin(); it != rawbuffs.end(); ++it) {
    total += rawbuffs_quantity((*it).first);
  }
  return total;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructStreamBlender(cyclus::Context* ctx) {
  return new StreamBlender(ctx);
}

} // namespace streamblender
