#ifndef CYCLUS_STREAMBLENDERS_STREAMBLENDER_FACILITY_H_
#define CYCLUS_STREAMBLENDERS_STREAMBLENDER_FACILITY_H_

#include <string>

#include "cyclus.h"

namespace streamblender {


/// @class StreamblenderFacility
///
/// This Facility is intended
/// as a skeleton to guide the implementation of new Facility
/// agents.
/// The StreamblenderFacility class inherits from the Facility class and is
/// dynamically loaded by the Agent class when requested.
///
/// @section introduction Introduction
/// The StreamBlender is a facility that receives commodities, holds onto them
/// for some number of months, offers them to the market of the new commodity. It
/// has three  stocks areas which hold commods of commodities: reserves,
/// processing, and  stocks. Incoming commodity orders are placed into reserves,
/// from which the  processing area is populated. When a process (some number of
/// months spent waiting)  has been completed, the commodity is converted and
/// moved into stocks. Requests for  commodities are bid upon based on the state
/// of the commodities in the stocks.
///
/// @section agentparams Agent Parameters
/// Place a description of the required input parameters which define the
/// agent implementation.
///   #. process_time_ : the number of timesteps a conversion process takes <0>
///   #. refuel_time : the number of timesteps required to reload the processing after
///   a process has finished <0>
///
/// The StreamBlender also maintains a cyclus::CommodityRecipeContext, which
/// allows it to track incommodity-inrecipe/outcommodity-outrecipe groupings.
///
/// @section optionalparams Optional Parameters
/// Place a description of the optional input parameters to define the
/// agent implementation.
///
/// @section detailed Detailed Behavior
/// After a StreamBlender enters the simulation, it will begin requesting all
/// incommodities.
///
/// As soon as it receives a commodity, that commodity is placed in the
/// processing storage area.
///
/// On the tick of the timestep in which that incommodity's time is up, it is
/// converted to the outcommodity type, by simply changing the commodity name.
/// Then, it is offered to the  outcommodity market.
///
/// The StreamBlender can manage multiple input-output commodity pairs, and keeps
/// track of the pair that each resource belongs to. Resources move through the
/// system independently of their input/output commodity types, but when they
/// reach the stocks area, they are offered as bids depedent on their output
/// commodity type.
///
/// @section requests Requests
/// A StreamBlender will make as many requests as it has possible input
/// commodities. It provides a constraint based on a total request amount
/// determined by its processing capacity.
///
/// @section bids Bids
/// A StreamBlender will bid on any request for any of its out_commodities, as
/// long as there is a positive quantity of material in its stocks area
/// associated with that output commodity.
///
/// @section ics Initial Conditions
/// A StreamBlender can be deployed with any number of commods in its reserve,
/// processing, and stocks buffers. Recipes and commodities for each of these
/// groupings must be specified.
///
/// @todo add decommissioning behavior if material is still in stocks
///
/// @section end End of Life
/// If the current time step is equivalent to the facility's lifetime, the
/// reactor will move all material in its processing to its stocks containers,
/// converted or not.
///
class StreamblenderFacility : public cyclus::Facility  {
 public:  
  /// Constructor for StreamblenderFacility Class
  /// @param ctx the cyclus context for access to simulation-wide parameters
  explicit StreamblenderFacility(cyclus::Context* ctx);

  /// The Prime Directive
  /// Generates code that handles all input file reading and restart operations
  /// (e.g., reading from the database, instantiating a new object, etc.).
  /// @warning The Prime Directive must have a space before it! (A fix will be
  /// in 2.0 ^TM)
  
  #pragma cyclus decl

  #pragma cyclus note {"doc": "A streamblender facility blends incoming streams "\
                              "of material into a goal recipe, by order of "\
                              "commodity  preference. Useful for Fuel Fabrication."}

  /// A verbose printer for the StreamblenderFacility
  virtual std::string str();
  
  /// The handleTick function specific to the StreamblenderFacility.
  /// @param time the time of the tick  
  virtual void Tick();

  /// The handleTick function specific to the StreamblenderFacility.
  /// @param time the time of the tock
  virtual void Tock();

  /// @brief The StreamblenderFacility requests materials
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> GetMatlRequests();

  /// @brief The StreamblenderFacility places accepted trade Materials in their
  /// Inventory
  virtual void AcceptMatlTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  /// @brief Responds to each request for this facility's commodity.  If a given
  /// request is more than this facility's inventory capacity, it will
  /// offer its minimum of its capacities.
  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
      GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                  commod_requests);

  /// @brief respond to each trade with a material enriched to the appropriate
  /// level given this facility's inventory
  ///
  /// @param trades all trades in which this trader is the supplier
  /// @param responses a container to populate with responses to each trade
  virtual void GetMatlTrades(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
    cyclus::Material::Ptr> >& responses);

  /* --- */

  /* --- StreamblenderFacility Members --- */

  /* --- */

 protected:
  /// @brief adds a material into the incoming commodity inventory
  /// @param commod the commodity name associated with the material
  /// @param mat the material that is incoming.  
  void AddMat_(std::string commod, cyclus::Material::Ptr mat);

  /// @brief gathers information about bids
  cyclus::BidPortfolio<cyclus::Material>::Ptr GetBids_(
        cyclus::CommodMap<cyclus::Material>::type& commod_requests,
        std::string commod,
        cyclus::toolkit::ResourceBuff* buffer);

  /// @brief suggests, based on the buffer, a material response to an offer
  cyclus::Material::Ptr TradeResponse_(
      double qty,
      cyclus::toolkit::ResourceBuff* buffer);

  /// @brief Move all unprocessed inventory to processing
  void BeginProcessing_();
 
  /// @brief This part does the blending. 
  cyclus::toolkit::ResourceBuff MeetNeed_(int iso, int n); 

  /// @brief number of possible goal recipes based on the available material
  int NPossible_();

  /// @brief collapse a resourcebuff into a single material
  cyclus::Material::Ptr CollapseBuff(cyclus::toolkit::ResourceBuff to_collapse);

  /// @brief move a resourcebuff of blended materials into the stocks
  void MoveToStocks_(cyclus::toolkit::ResourceBuff blended_buff, int n_poss);

  /// @brief calculates the total mass of the goal material composition [kg]
  double GoalCompMass_();

  /// @brief calculates goal material composition
  cyclus::CompMap GoalCompMap_();

  /// @brief calculates goal material composition
  cyclus::Composition::Ptr GoalComp_();

  /// @brief returns the preferred commodity sources for the isotope, iso
  std::set<std::string> prefs(int iso);

  /// @brief make as much of the goal mat as possible with ready materials.
  void BlendStreams_();

  /// @brief gives current quantity of commod in inventory
  const double inventory_quantity(std::string commod) const;

  /// @brief gives current quantity of all commods in inventory
  const double inventory_quantity() const;

  /// @brief this facility's commodity-recipe context
  inline void crctx(const cyclus::toolkit::CommodityRecipeContext& crctx) {
    crctx_ = crctx;
  }
  inline cyclus::toolkit::CommodityRecipeContext crctx() const {
    return crctx_;
  }

  /// @brief returns the time key for ready materials
  int ready(){ return context()->time() - process_time ; }

  /* --- Module Members --- */
  #pragma cyclus var {"tooltip":"input commodities",\
                      "doc":"list of commodities accepted by this facility"}
  std::vector< std::string > in_commods;
  inline std::vector< std::string > in_commods_() const {return in_commods;};

  #pragma cyclus var {"tooltip":"output commodity",\
                      "doc":"commodity produced by this facility"}
  std::string out_commod;
  inline std::string out_commod_() const {return out_commod;};

  #pragma cyclus var {"tooltip":"input recipes",\
                      "doc":"a list of recipes accepted by this facility"}
  std::vector< std::string > in_recipes;
  inline std::vector< std::string > in_recipes_() const {return in_recipes;};

  #pragma cyclus var {"tooltip":"output recipe",\
                      "doc":"recipe produced by this facility"}
  std::string out_recipe;
  inline std::string out_recipe_() const {return out_recipe;};

  #pragma cyclus var {"default": 0,\
                      "tooltip":"process time (timesteps)",\
                      "doc":"the time it takes to convert a received commodity (timesteps)."}
  int process_time; //should be nonnegative

  #pragma cyclus var {"default": 1e299,\
                      "tooltip":"maximum inventory size (kg)",\
                      "doc":"the amount of material that can be in storage at "\
                      "one time (kg)."}
  double max_inv_size; //should be nonnegative

  #pragma cyclus var{'capacity': 'max_inv_size'}
  std::map<std::string, cyclus::toolkit::ResourceBuff> inventory;
  cyclus::toolkit::ResourceBuff stocks;
  cyclus::toolkit::ResourceBuff wastes;

  /// @brief a list of preffered commodities
  std::map<int, std::set<std::string> > prefs_;

  /// @brief map from ready time to resource buffers
  std::map<int, std::map<std::string, 
    cyclus::toolkit::ResourceBuff> > processing;

  cyclus::toolkit::CommodityRecipeContext crctx_;

  /// @brief the processing time required for a full process
  inline void process_time_(int t) { process_time = t; }
  inline int process_time_() const { return process_time; }

  /// @brief the maximum amount allowed in inventory
  inline void capacity(double c) { max_inv_size = c; }
  inline double capacity() const { return max_inv_size; }

  /// @brief current maximum amount that can be added to processing
  inline double current_capacity() const {
    return (max_inv_size - inventory_quantity()); } 
  
  friend class StreamblenderFacilityTest;
};

}  // namespace streamblender

#endif  // CYCLUS_STREAMBLENDERS_STREAMBLENDER_FACILITY_H_
