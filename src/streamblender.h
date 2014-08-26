#ifndef CYCLUS_STREAMBLENDERS_STREAMBLENDER_H_
#define CYCLUS_STREAMBLENDERS_STREAMBLENDER_H_

#include <string>

#include "cyclus.h"

namespace streamblender {


/// @class StreamBlender
///
/// This holds a module for cyclus that blends streams of material into a 
/// resulting material. It aims for a goal recipe and can specify an order of 
/// preference for using available commodities. It was designed to run as a fuel 
/// fabrication facility, but can be used for any purpose that requires a 
/// combining of commodities that represent material streams. It was initially 
/// developed to run the fco code-to-code comparison.
///
/// The StreamBlender class inherits from the Facility class and is
/// dynamically loaded by the Agent class when requested.
///
/// @section introduction Introduction
/// The StreamBlender is a facility that receives commodities, holds onto them
/// for some number of months, offers them to the market of the new commodity. It
/// has three  blendbuff areas which hold commods of commodities: reserves,
/// processing, and  blendbuff. Incoming commodity orders are placed into reserves,
/// from which the  processing area is populated. When a process (some number of
/// months spent waiting)  has been completed, the commodity is converted and
/// moved into blendbuff. Requests for  commodities are bid upon based on the state
/// of the commodities in the blendbuff.
///
/// @section agentparams Agent Parameters
/// Place a description of the required input parameters which define the
/// agent implementation.
///   #. process_time_ : the number of timesteps a conversion process takes <0>
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
/// reach the blendbuff area, they are offered as bids depedent on their output
/// commodity type.
///
/// @section requests Requests
/// A StreamBlender will make as many requests as it has possible input
/// commodities. It provides a constraint based on a total request amount
/// determined by its processing capacity.
///
/// @section bids Bids
/// A StreamBlender will bid on any request for any of its out_commodities, as
/// long as there is a positive quantity of material in its blendbuff area
/// associated with that output commodity.
///
/// @section ics Initial Conditions
/// A StreamBlender can be deployed with any number of commods in its reserve,
/// processing, and blendbuff buffers. Recipes and commodities for each of these
/// groupings must be specified.
///
/// @todo add decommissioning behavior if material is still in blendbuff
///
/// @section end End of Life
/// If the current time step is equivalent to the facility's lifetime, the
/// reactor will move all material in its processing to its blendbuff containers,
/// converted or not.
///
class StreamBlender : 
  public cyclus::Facility, 
  public cyclus::toolkit::CommodityProducer {
 public:  
  /// Constructor for StreamBlender Class
  /// @param ctx the cyclus context for access to simulation-wide parameters
  explicit StreamBlender(cyclus::Context* ctx);

  /// The Prime Directive
  /// Generates code that handles all input file reading and restart operations
  /// (e.g., reading from the database, instantiating a new object, etc.).
  /// @warning The Prime Directive must have a space before it! (A fix will be
  /// in 2.0 ^TM)
  
  #pragma cyclus decl

  #pragma cyclus note {"doc": "A streamblender facility blends incoming streams "\
                              "of material into a goal recipe, by order of "\
                              "commodity  preference. Useful for Fuel Fabrication."}

  /// A verbose printer for the StreamBlender
  virtual std::string str();

  /// When this facility enters the simulation, register it as a commodity 
  /// producer.
  virtual void EnterNotify();
  
  /// The handleTick function specific to the StreamBlender.
  /// @param time the time of the tick  
  virtual void Tick();

  /// The handleTick function specific to the StreamBlender.
  /// @param time the time of the tock
  virtual void Tock();

  /// @brief The StreamBlender requests materials
  virtual std::set<cyclus::RequestPortfolio<cyclus::Material>::Ptr> GetMatlRequests();

  /// @brief The StreamBlender places accepted trade Materials in their
  /// Inventory
  virtual void AcceptMatlTrades(
      const std::vector< std::pair<cyclus::Trade<cyclus::Material>,
      cyclus::Material::Ptr> >& responses);

  /// @brief Responds to each request for this facility's commodity.  If a given
  /// request is more than this facility's rawbuffs capacity, it will
  /// offer its minimum of its capacities.
  virtual std::set<cyclus::BidPortfolio<cyclus::Material>::Ptr>
      GetMatlBids(cyclus::CommodMap<cyclus::Material>::type&
                  commod_requests);

  /// @brief respond to each trade with a material enriched to the appropriate
  /// level given this facility's rawbuffs
  ///
  /// @param trades all trades in which this trader is the supplier
  /// @param responses a container to populate with responses to each trade
  virtual void GetMatlTrades(
    const std::vector< cyclus::Trade<cyclus::Material> >& trades,
    std::vector<std::pair<cyclus::Trade<cyclus::Material>,
    cyclus::Material::Ptr> >& responses);

  /* --- */

  /* --- StreamBlender Members --- */

  /* --- */

 protected:
  /// @brief adds a material into the incoming commodity rawbuffs
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

  /// @brief Move all unprocessed rawbuffs to processing
  void BeginProcessing_();
 
  /// @brief This part does the blending. 
  cyclus::toolkit::ResourceBuff MeetNeed_(int iso, int n); 

  /// @brief number of possible goal recipes based on the available material
  int NPossible_();

  /// @brief amount of iso available in this map of buffers
  /// @param source_buffs map of commodities-resourcebuffers that holds source mats
  /// @param iso the isotope for which to find availability 
  double AmtPossible_(std::map< std::string, cyclus::toolkit::ResourceBuff > 
    source_buffs, int iso );

  /// @brief amount of iso available in this buffer
  /// @param buff a resourcebuffer that holds source mats
  /// @param iso the isotope for which to find availability 
  double AmtPossible_(cyclus::toolkit::ResourceBuff* buff, int iso );

  /// @brief collapse a resourcebuff into a single material
  cyclus::Material::Ptr CollapseBuff(cyclus::toolkit::ResourceBuff* to_collapse);

  /// @brief move a resourcebuff of blended materials into the blendbuff
  void Blend_(cyclus::toolkit::ResourceBuff* blended_buff, int n_poss);

  /// @brief calculates the total mass of the goal material composition [kg]
  double GoalCompMass_();

  /// @brief calculates goal material composition
  cyclus::CompMap GoalCompMap_();

  /// @brief calculates goal material composition
  cyclus::Composition::Ptr GoalComp_();

  std::set<std::string> prefs(int iso);

  /// @brief returns the indices where the isotope appears in the isos list
  std::set<int> IsoIdx_(int iso); 

  /// @brief returns the set of sources for this iso
  std::set<std::string> Sources_(int iso); 

  /// @brief returns the map made from the isos and sources
  std::map< int, std::set<std::string> > Prefs_(); 

  /// @brief make as much of the goal mat as possible with ready materials.
  void BlendStreams_();

  /// @brief gives current quantity of commod in rawbuffs
  const double rawbuffs_quantity(std::string commod) const;

  /// @brief gives current quantity of all commods in rawbuffs
  const double rawbuffs_quantity() const;

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
                      "tooltip":"maximum rawbuffs size (kg)",\
                      "doc":"the amount of material that can be in storage at "\
                      "one time (kg)."}
  double max_inv_size; //should be nonnegative

  #pragma cyclus var {"default": 1e299,\
                      "tooltip":"maximum processing rate (kg)",\
                      "doc":"the amount of material that can be processed per "\
                      "timestep (kg)."}
  double capacity; //should be nonnegative

  #pragma cyclus var {"default": 0,\
                      "tooltip":"cost per unit",\
                      "doc":"cost per unit out_commod (kg)."}
  double cost; //should be nonnegative

  #pragma cyclus var {"tooltip":"The isotopes of interest for stream blending",\
                      "doc":"This list can have repeated entries and should be "\
                      "matched to the sources list."}
  std::vector<int> isos;

  #pragma cyclus var {"tooltip":"The source commodities for each isotope",\
                      "doc":"This list cannot have repeated entries. It "\
                      "should be matched to the isos list "}
  std::vector<std::string> sources;

  std::map<std::string, cyclus::toolkit::ResourceBuff> rawbuffs;
  cyclus::toolkit::ResourceBuff blendbuff;
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

  /// @brief the maximum amount allowed in rawbuffs
  inline void max_inv_size_(double m) { max_inv_size = m; }
  inline double max_inv_size_() const { return max_inv_size; }

  /// @brief the maximum amount allowed in rawbuffs
  inline void capacity_(double c) { capacity = c; }
  inline double capacity_() const { return capacity; }

  /// @brief current maximum amount that can be added to processing
  inline double current_capacity() const {
    return (max_inv_size - blendbuff.quantity()); } 
  
  friend class StreamBlenderTest;
};

}  // namespace streamblender

#endif  // CYCLUS_STREAMBLENDERS_STREAMBLENDER_H_
