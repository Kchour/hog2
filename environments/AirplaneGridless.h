//
//  AirplaneGridless.h
//  hog2 glut
//
//  Created by Thayne Walker on 2/6/16.
//  Copyright © 2017 University of Denver. All rights reserved.
//

#ifndef AirplaneGridless_h
#define AirplaneGridless_h

#include <vector>
#include <cassert>
#include <cmath>
#include "SearchEnvironment.h"
#include "AirplanePerimeterDBBuilder.h"
#include "constants.h"

struct PlatformAction {
public:
	PlatformAction(double t=0, double p=0, int8_t s=0)
          :turnHalfDegs(int8_t(t*2.0)),
           pitchHalfDegs(int8_t(p*2.0)),
           speed(s) {}

	int8_t turnHalfDegs; // turn in half-degrees
	int8_t pitchHalfDegs; // pitch in half-degrees
	int8_t speed; // speed increment (+/-1)

	double turn() const { return double(turnHalfDegs)/2.0; }
	void turn(double val){ turnHalfDegs=int8_t(round(val*2.0)); }
	double pitch() const { return double(pitchHalfDegs)/2.0; }
	void pitch(double val){ pitchHalfDegs=int8_t(round(val*2.0)); }
};


/** Output the information in an Platform action */
static std::ostream& operator <<(std::ostream & out, PlatformAction const& act)
{
	out << "(turn:" << act.turn() << " pitch:" << act.pitch() << " speed: " << signed(act.speed) << ")";
	return out;
}


// state
struct PlatformState {
        static const double SPEEDS[];
        static const double TIMESTEP;
	// Constructors
	PlatformState() :x(0),y(0),z(0),t(0),headingHalfDegs(360),rollHalfDegs(180),pitchHalfDegs(180),speed(3){}
	PlatformState(float lt,float ln, float a, double h, double p, int8_t s, uint32_t time=0) :
		x(lt),y(ln),z(a),t(time),headingHalfDegs(360.+h*2),rollHalfDegs(180),pitchHalfDegs((p+90)*2.0),speed(s){}

	double hdg()const{return double(headingHalfDegs)/2.0-180.;}
	double roll()const{return double(rollHalfDegs)/2.0-90.0;}
	double pitch()const{return double(pitchHalfDegs)/2.0-90.0;}

        // Set the roll based on current turn rate.
        void setRoll(double turnDegsPerSecond){rollHalfDegs=(atan((turnDegsPerSecond*constants::degToRad*SPEEDS[speed])/constants::gravitationalConstant)*constants::radToDeg + 90.0) * 2.0;}

	uint64_t key() const {
		// We have too many bits to fit in the key (32+32+32+16+8+8)=128
		uint32_t l1(*((uint32_t*)&x));
		uint32_t l2(*((uint32_t*)&y));
		uint32_t l3(*((uint32_t*)&z));
		//uint32_t l4(*((uint32_t*)&t));

                uint64_t h1(l1);
		h1 = h1 << 32;
		h1 |= l2;

                uint64_t h2(l3);
                h2 = h2 << 32;
                //h2 |= l4;
                h2 |= t;

                uint64_t h3(headingHalfDegs);
		h3 = h3 << 9;
		h3 |= rollHalfDegs & (0x1ff); // we only care about the range 0-360 (we normalize to +/- 90)
		h3 = h3 << 9;
		h3 |= pitchHalfDegs & (0x1ff); // we only care about the range 0-360 (we normalize to +/- 90)
                h3 = h3 << 3;
                h3 |= speed & (0x7);

		// Put the 64 bit hashes together using a prime product hash
		return (h1 * 16777619) ^ h2 ^ (h3 * 4194319);
	}

        void operator += (PlatformAction const& other){
          headingHalfDegs+=other.turnHalfDegs;
          pitchHalfDegs+=other.pitchHalfDegs;
          setRoll(other.turn());
          speed += other.speed;
          //std::cout << "yaw " << hdg() << " " << headingHalfDegs << "\n";
          //std::cout << "x. " << cos((90.-hdg())*constants::degToRad) << "\n";
          //std::cout << "y. " << sin((90.-hdg())*constants::degToRad) << "\n";
          //std::cout << "y+ " << sin((90.-hdg())*constants::degToRad)*SPEEDS[speed]*TIMESTEP << "\n";
          //std::cout << "z. " << cos((90.-pitch())*constants::degToRad) << "\n";
          //std::cout << " s " << SPEEDS[speed] << " " << SPEEDS[speed]*TIMESTEP << "\n";
          x+=cos((90.-hdg())*constants::degToRad)*SPEEDS[speed]*TIMESTEP;
          y+=sin((90.-hdg())*constants::degToRad)*SPEEDS[speed]*TIMESTEP;
          z+=cos((90.-pitch())*constants::degToRad)*SPEEDS[speed]*TIMESTEP;
          t++;
        }

        void operator -= (PlatformAction const& other){
          //std::cout << "_yaw " << hdg() << " " << headingHalfDegs << "\n";
          //std::cout << "_x. " << cos((90.-hdg())*constants::degToRad) << "\n";
          //std::cout << "_y. " << sin((90.-hdg())*constants::degToRad) << "\n";
          //std::cout << "_y+ " << sin((90.-hdg())*constants::degToRad)*SPEEDS[speed]*TIMESTEP << "\n";
          //std::cout << "_z. " << cos((90.-pitch())*constants::degToRad) << "\n";
          //std::cout << " _s " << SPEEDS[speed] << " " << SPEEDS[speed]*TIMESTEP << "\n";
          x-=cos((90.-hdg())*constants::degToRad)*SPEEDS[speed]*TIMESTEP;
          y-=sin((90.-hdg())*constants::degToRad)*SPEEDS[speed]*TIMESTEP;
          z-=cos((90.-pitch())*constants::degToRad)*SPEEDS[speed]*TIMESTEP;
          t--;
          headingHalfDegs-=other.turnHalfDegs;
          pitchHalfDegs-=other.pitchHalfDegs;
          setRoll(other.turn());
          speed -= other.speed;
        }

        bool operator == (PlatformState const& other)const{
          return fequal(x,other.x) && fequal(y,other.y) && fequal(z,other.z)
            && (speed==other.speed) && (headingHalfDegs==other.headingHalfDegs)
            && (pitchHalfDegs==other.pitchHalfDegs);
        }

	// Fields
	float x;
	float y;
	float z;
	uint32_t t;
	uint16_t headingHalfDegs;
	int16_t rollHalfDegs;
	int16_t pitchHalfDegs;
	int8_t speed;  // 5 speeds: 1=100mps, 2=140, 3=180, 4=220, 5=260 mps
};


/** Output the information in a Platform state */
static std::ostream& operator <<(std::ostream & out, PlatformState const& loc)
{
	out << "(x:" << loc.x << ", y:" << loc.y << ", z:" << loc.z << ", h: " << loc.hdg() << ", r: " << signed(loc.roll()) << ", s: " << unsigned(loc.speed) << ", t: " << loc.t << ")";
	//out << "val<-cbind(val,c(" << loc.x << "," << loc.y << "," << loc.alt() << "," << loc.heading() << "," << signed(loc.roll) << "," << loc.sum << "," << loc.depth << "))";
	return out;
}

bool operator==(PlatformAction const& a1, PlatformAction const& a2);


struct gridlessLandingStrip {
	gridlessLandingStrip(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2, PlatformState &launch_state, PlatformState &landing_state, PlatformState &goal_state) : x1(x1), x2(x2), y1(y1), y2(y2), 
				 launch_state(launch_state), landing_state(landing_state), goal_state(goal_state) {}
	uint16_t x1;
	uint16_t x2;
	uint16_t y1;
	uint16_t y2;
	uint16_t z = 0;
	PlatformState goal_state;
	PlatformState launch_state;
	PlatformState landing_state;
};


// Actual Environment
class AirplaneGridlessEnvironment : public SearchEnvironment<PlatformState, PlatformAction>
{
  public:

    // Constructor
    AirplaneGridlessEnvironment(
        unsigned width=80,
        unsigned length=80,
        unsigned height=20,
        double climbRate=5,
        double minSpeed=1,
        double maxSpeed=5,
        uint8_t numSpeeds=5, // Number of discrete speeds
        double cruiseBurnRate=.006, // Fuel burn rate in liters per unit distance
        double speedBurnDelta=0.001, // Extra fuel cost for non-cruise speed
        double climbCost=0.001, // Fuel cost for climbing
        double descendCost=-0.0005, // Fuel cost for descending
        double gridSize=30.0); // Horizontal grid width (meters)
    //std::string const& perimeterFile=std::string("airplanePerimeter.dat"));

    virtual char const*const name()const{return "AirplaneGridlessEnvironment";}
    // Successors and actions
    virtual void GetSuccessors(const PlatformState &nodeID, std::vector<PlatformState> &neighbors) const;
    virtual void GetReverseSuccessors(const PlatformState &nodeID, std::vector<PlatformState> &neighbors) const;

    virtual void GetActions(const PlatformState &nodeID, std::vector<PlatformAction> &actions) const;


    virtual void GetReverseActions(const PlatformState &nodeID, std::vector<PlatformAction> &actions) const;


    virtual void ApplyAction(PlatformState &s, PlatformAction dir) const;
    virtual void UndoAction(PlatformState &s, PlatformAction dir) const;
    virtual void GetNextState(const PlatformState &currents, PlatformAction dir, PlatformState &news) const;
    virtual bool InvertAction(PlatformAction &a) const { return false; }
    virtual PlatformAction GetAction(const PlatformState &node1, const PlatformState &node2) const;
    virtual PlatformAction GetReverseAction(const PlatformState &node1, const PlatformState &node2) const;


    // Occupancy Info not supported
    virtual OccupancyInterface<PlatformState,PlatformAction> *GetOccupancyInfo() { return 0; }

    // Heuristics and paths
    virtual double HCost(const PlatformState &node1, const PlatformState &node2) const;
    virtual double ReverseHCost(const PlatformState &,const PlatformState &)  const;
    virtual double ReverseGCost(const PlatformState &node1, const PlatformState &node2) const;
    virtual double HCost(const PlatformState &)  const { assert(false); return 0; }
    virtual double GCost(const PlatformState &node1, const PlatformState &node2) const;
    virtual double GCost(const PlatformState &node1, const PlatformAction &act) const;
    virtual double GetPathLength(const std::vector<PlatformState> &n) const;

    // Goal testing
    virtual bool GoalTest(const PlatformState &node, const PlatformState &goal) const;
    virtual bool GoalTest(const PlatformState &) const { assert(false); return false; }

    // Hashing
    virtual PlatformState GetState(uint64_t hash) const;
    virtual uint64_t GetStateHash(const PlatformState &node) const;
    virtual uint64_t GetActionHash(PlatformAction act) const;

    // Drawing
    virtual void OpenGLDraw() const;
    virtual void OpenGLDraw(const PlatformState &l) const;
    virtual void OpenGLDraw(const PlatformState& oldState, const PlatformState &newState, float perc) const;
    virtual void OpenGLDraw(const PlatformState &, const PlatformAction &) const;
    void GLDrawLine(const PlatformState &a, const PlatformState &b) const;
    void GLDrawPath(const std::vector<PlatformState> &p, const std::vector<PlatformState> &wpts) const;
    void DrawAirplane() const;
    void DrawQuadCopter() const;
    recVec GetCoordinate(int x, int y, int z) const;

    // Getters
    std::vector<uint8_t> getGround();
    std::vector<recVec> getGroundNormals();

    std::vector<PlatformAction> getInternalActions();

    // Landing Strups
    virtual void AddLandingStrip(gridlessLandingStrip & x);
    virtual const std::vector<gridlessLandingStrip>& GetLandingStrips() const {return landingStrips;}

    // State information
    const uint8_t numSpeeds;  // Number of speed steps
    const double minSpeed;    // Meters per time step
    const double maxSpeed;    // Meters per time step
    double const gridSize;    // 30 meters

    PlatformState const* goal;
    PlatformState const& getGoal()const{return *goal;}
    void setGoal(PlatformState const& g){goal=&g;}

    PlatformState const* start;
    PlatformState const& getStart()const{return *start;}
    void setStart(PlatformState const& s){start=&s;}

  protected:

    virtual AirplaneGridlessEnvironment& getRef() {return *this;}
    void SetGround(int x, int y, uint8_t val);
    uint8_t GetGround(int x, int y) const;
    bool Valid(int x, int y);
    recVec &GetNormal(int x, int y);
    recVec GetNormal(int x, int y) const;
    void RecurseGround(int x1, int y1, int x2, int y2);
    const int width;
    const int length;
    const int height;
    std::vector<uint8_t> ground;
    std::vector<recVec> groundNormals;
    void DoNormal(recVec pa, recVec pb) const;
    mutable std::vector<PlatformAction> internalActions;

    std::vector<gridlessLandingStrip> landingStrips;

    const double climbRate;      //Meters per time step
    // Assume 1 unit of movement to be 3 meters
    // 16 liters per hour/ 3600 seconds / 22 mps = 0.0002 liters per meter
    double const cruiseBurnRate;//0.0002*30.0 liters per unit
    double const speedBurnDelta;//0.0001 liters per unit
    double const climbCost;//1.0475;
    double const descendCost;

    // Caching for turn information
    std::vector<int8_t> turns;
    std::vector<int8_t> quad_turns;

  private:
    virtual double myHCost(const PlatformState &node1, const PlatformState &node2) const;
    //bool perimeterLoaded;
    //std::string perimeterFile;
    //AirplanePerimeterDBBuilder<PlatformState, PlatformAction, AirplaneGridlessEnvironment> perimeter[2]; // One for each type of aircraft

    //TODO Add wind constants
    //const double windSpeed = 0;
    //const double windDirection = 0;
};

#endif /* Airplane_h */
