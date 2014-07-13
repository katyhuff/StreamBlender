#include "streamblender_facility.h"

namespace streamblender {

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
StreamblenderFacility::StreamblenderFacility(cyclus::Context* ctx)
    : cyclus::Facility(ctx) {};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// pragmas

#pragma cyclus def schema streamblender::StreamblenderFacility

#pragma cyclus def annotations streamblender::StreamblenderFacility

#pragma cyclus def initinv streamblender::StreamblenderFacility

#pragma cyclus def snapshotinv streamblender::StreamblenderFacility

#pragma cyclus def initfromdb streamblender::StreamblenderFacility

#pragma cyclus def initfromcopy streamblender::StreamblenderFacility

#pragma cyclus def infiletodb streamblender::StreamblenderFacility

#pragma cyclus def snapshot streamblender::StreamblenderFacility

#pragma cyclus def clone streamblender::StreamblenderFacility

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::string StreamblenderFacility::str() {
  std::stringstream ss;
  ss << cyclus::Facility::str();
  ss << " has facility parameters {" << "\n"
     << "     Input Commodity = " << in_commod_() << ",\n"
     << "     Output Commodity = " << out_commod_() << ",\n"
     << "     Process Time = " << process_time_() << ",\n"
     << "     Capacity = " << capacity() << ",\n"
     << "'}";
  return ss.str();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamblenderFacility::Tick() {
  LOG(cyclus::LEV_INFO3, "SBlend") << prototype() << " is ticking {";

  // @TODO uncomment and implement
  //if (context()->time() == lifetime_) {
  //  EndLife_();
  //}

  LOG(cyclus::LEV_INFO3, "SBlend") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamblenderFacility::Tock() {
  LOG(cyclus::LEV_INFO3, "SBlend") << prototype() << " is tocking {";
  BeginProcessing_(); // place unprocessed inventory into processing

  std::set<std::string>::const_iterator it;
  for (it = crctx_.in_commods().begin(); it != crctx_.in_commods().end(); ++it) {
    BlendStreams_();
  }

  LOG(cyclus::LEV_INFO3, "SBlend") << "}";
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr>
StreamblenderFacility::GetMatlRequests() {
  using cyclus::CapacityConstraint;
  using cyclus::Material;
  using cyclus::RequestPortfolio;
  using cyclus::Request;

  std::set<RequestPortfolio<Material>::Ptr> ports;
  RequestPortfolio<Material>::Ptr port(new RequestPortfolio<Material>());
  Material::Ptr mat = Request_();
  double amt = mat->quantity();

  if (amt > cyclus::eps()) {
    CapacityConstraint<Material> cc(amt);
    port->AddConstraint(cc);

    port->AddRequest(mat, this, in_commod_());

    ports.insert(port);
  }

  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamblenderFacility::AcceptMatlTrades(
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
    if (mat_commods.count(commod) == 0) {
      mat_commods[commod] = mat;
    } else {
      mat_commods[commod]->Absorb(mat);
    }
  }

  // add each blob to reserves
  std::map<std::string, Material::Ptr>::iterator it;
  for (it = mat_commods.begin(); it != mat_commods.end(); ++it) {
    AddMat_(it->first, it->second);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
StreamblenderFacility::GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                          commod_requests) {
  using cyclus::BidPortfolio;
  using cyclus::Material;

  std::set<BidPortfolio<Material>::Ptr> ports;

  std::set<std::string>::const_iterator it;
  BidPortfolio<Material>::Ptr port = GetBids_(commod_requests,
                                              out_commod_(),
                                              &stocks);
  if (!port->bids().empty()) {
    ports.insert(port);
  }

  return ports;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamblenderFacility::GetMatlTrades(
  const std::vector< cyclus::Trade<cyclus::Material> >& trades,
  std::vector<std::pair<cyclus::Trade<cyclus::Material>,
  cyclus::Material::Ptr> >& responses) {
  using cyclus::Material;
  using cyclus::Trade;

  // for each trade, respond
  std::vector< Trade<Material> >::const_iterator it;
  for (it = trades.begin(); it != trades.end(); ++it) {
    std::string commodity = it->request->commodity();
    double qty = it->amt;
    // create a material pointer representing what you can offer
    Material::Ptr response = TradeResponse_(qty, &stocks);

    responses.push_back(std::make_pair(*it, response));
    LOG(cyclus::LEV_INFO5, "SBlend") << prototype()
                                  << " just received an order"
                                  << " for " << it->amt
                                  << " of " << commodity;
  }

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamblenderFacility::AddMat_(std::string commod, 
    cyclus::Material::Ptr mat) {

  LOG(cyclus::LEV_INFO5, "SBlend") << prototype() << " is initially holding "
                                << inventory_quantity() << " total.";

  try {
    inventory[commod].Push(mat);
  } catch (cyclus::Error& e) {
    e.msg(Agent::InformErrorMsg(e.msg()));
    throw e;
  }

  LOG(cyclus::LEV_INFO5, "SBlend") << prototype() << " added " << mat->quantity()
                                << " of " << in_commod_()
                                << " to its inventory, which is holding "
                                << inventory_quantity() << " total.";

}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Material::Ptr StreamblenderFacility::Request_() {
  double qty = std::max(0.0, current_capacity());
  return cyclus::Material::CreateUntracked(qty,
                                        context()->GetRecipe(in_recipe));
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::BidPortfolio<cyclus::Material>::Ptr StreamblenderFacility::GetBids_(
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
cyclus::Material::Ptr StreamblenderFacility::TradeResponse_(
    double qty,
    cyclus::toolkit::ResourceBuff* buffer) {
  using cyclus::Material;
  using cyclus::ResCast;

  std::vector<Material::Ptr> manifest;
  try {
    // pop amount from inventory and blob it into one material
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
void StreamblenderFacility::BeginProcessing_(){
  LOG(cyclus::LEV_DEBUG2, "SBlend") << "CommodConverter " << prototype()
                                    << " added resources to processing";
  std::map<std::string, cyclus::toolkit::ResourceBuff>::iterator it;
  for (it = inventory.begin(); it != inventory.end(); ++it){
    while (!(*it).second.empty()){
      try {
        processing[context()->time()][(*it).first].Push((*it).second.Pop());
      } catch(cyclus::Error& e) {
        e.msg(Agent::InformErrorMsg(e.msg()));
        throw e;
      }
    }
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::toolkit::ResourceBuff StreamblenderFacility::MeetNeed_(int iso, int n){
  using cyclus::toolkit::ResourceBuff;

  ResourceBuff iso_source_buff =  ResourceBuff();
  double need = n*GoalCompMap_()[iso];
  std::set<std::string>::const_iterator pref;
  std::set<std::string> preflist = prefs(iso);
  for(pref = preflist.begin(); pref != preflist.end(); ++pref){
      double avail = processing[ready()][*pref].quantity();
      double diff = need - avail;
      if( need > 0 && need > avail ){
        iso_source_buff.PushAll(processing[ready()][*pref].PopQty(avail));
        need = diff;
      } else if ( need > 0 && need <= avail ){
        iso_source_buff.PushAll(processing[ready()][*pref].PopQty(need));
        need = 0;
      }
  }
  return iso_source_buff;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int StreamblenderFacility::NPossible_(){

  bool first_run = true;
  int n_poss = 0;
  int prev = 0;
  std::map<int, double>::const_iterator it;
  cyclus::CompMap goal = GoalCompMap_();
  for(it = goal.begin(); it != goal.end(); ++it){
    int iso = it->first;
    double amt = it->second;
    double avail = 0;
    std::set<std::string>::const_iterator pref;
    std::set<std::string> preflist = prefs(iso);
    for(pref = preflist.begin(); pref != preflist.end(); ++pref){
      std::map< std::string, cyclus::toolkit::ResourceBuff >::iterator found;
      found = processing[ready()].find(*pref);
      bool isfound = (found!=processing[ready()].end());
      if(isfound){
        avail += processing[ready()][*pref].quantity();
      }
    }
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
cyclus::Material::Ptr StreamblenderFacility::CollapseBuff(cyclus::toolkit::ResourceBuff to_collapse){
  using cyclus::toolkit::Manifest;
  using cyclus::Material;
  using cyclus::ResCast;
  double qty =  to_collapse.quantity();
  Manifest manifest = to_collapse.PopQty(qty);


  Material::Ptr back = ResCast<Material>(manifest.back());
  manifest.pop_back();
  while ( !manifest.empty() ){
    back->Absorb(ResCast<Material>(manifest.back()));
    manifest.pop_back();
  }
  return back;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamblenderFacility::MoveToStocks_(cyclus::toolkit::ResourceBuff fabbed_fuel_buff, int n_poss){
  using cyclus::toolkit::Manifest;
  using cyclus::Material;
  using cyclus::ResCast;

  Material::Ptr soup = CollapseBuff(fabbed_fuel_buff);

  for( int i=0; i<n_poss; ++i){
    Material::Ptr goal_mat =  soup->ExtractComp(GoalCompMass_(), GoalComp_());
    stocks.Push(goal_mat);
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::Composition::Ptr StreamblenderFacility::GoalComp_(){
  std::string out = out_recipe_();
  cyclus::Composition::Ptr recipe = context()->GetRecipe(out_recipe_());
  return recipe;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
cyclus::CompMap StreamblenderFacility::GoalCompMap_(){
  return GoalComp_()->mass();
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double StreamblenderFacility::GoalCompMass_(){
  double amt = 0;
  std::map<int, double>::const_iterator it;
  cyclus::CompMap goal = GoalCompMap_();
  for(it=goal.begin(); it!=goal.end(); ++it){
    amt += it->second;
  }
  return amt;
}
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::set<std::string> StreamblenderFacility::prefs(int iso){

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

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void StreamblenderFacility::BlendStreams_(){
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

    MoveToStocks_(fabbed_fuel_buff, n);
    LOG(cyclus::LEV_DEBUG2, "SBlend") << "StreamblenderFacility " << prototype() 
                                     << " is blending streams.";
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double StreamblenderFacility::inventory_quantity(std::string commod)const {
  using cyclus::toolkit::ResourceBuff;

  std::map<std::string, ResourceBuff>::const_iterator found;
  found = inventory.find(commod);
  double amt;
  if ( found != inventory.end() ){
    amt = (*found).second.quantity();
  } else {
    amt =0;
  }
  return amt;
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const double StreamblenderFacility::inventory_quantity() const {
  using cyclus::toolkit::ResourceBuff;

  double total = 0;
  std::map<std::string, ResourceBuff>::const_iterator it;
  for( it = inventory.begin(); it != inventory.end(); ++it) {
    total += inventory_quantity((*it).first);
  }
  return total;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
extern "C" cyclus::Agent* ConstructStreamblenderFacility(cyclus::Context* ctx) {
  return new StreamblenderFacility(ctx);
}

} // namespace streamblender
