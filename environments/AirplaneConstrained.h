//
//  AirplaneConstrained.h
//  hog2 glut
//
//  Created by David Chan on 6/8/16.
//  Copyright (c) 2016 University of Denver. All rights reserved.
//
//  Modified by Thayne Walker 2017
//

#ifndef __hog2_glut__AirplaneConstrainedEnvironment__
#define __hog2_glut__AirplaneConstrainedEnvironment__

#include "Map.h"
#include "Airplane.h"
#include "ConstrainedEnvironment.h"
#include "UnitTimeCAT.h"
#include "PositionalUtils.h"
#include "MultiAgentStructures.h"

#include <cmath>
#include <memory>
#include <limits>

/**
 * Defines a constrained version of the airplane environment. Thus, we can add constraints 
 * consisting of airtimeStates and actions, which allow us to deal with the issues that
 * come up in CBS
 */
class AirplaneConstrainedEnvironment : public ConstrainedEnvironment<airtimeState,airplaneAction>
{
public:
 
	/// CONSTRUCTORS

	/** Construct a random-ground airplane environment which forms the backbone of the model */
	//AirplaneConstrainedEnvironment();
	/** Construct a constrained environment from an existing environment */
	AirplaneConstrainedEnvironment(AirplaneEnvironment* ae,int width=80,int length=80,int height=20);
        virtual std::string name()const{return ae->name();}

	/// CONSTRAINTS
	
	/** Add a constraint to the model */
	void AddSoftConstraint(SoftConstraint<airtimeState> const& c){sconstraints.push_back(c);}

	void AddStaticConstraint(Collision<airtimeState> const& c);
	void AddStaticPointConstraint(const airtimeState &loc);
	void AddStaticBoxConstraint(const airtimeState &loc1, const airtimeState &loc2);


	Collision<airtimeState> GetPointConstraint(const airtimeState &loc);
	Collision<airtimeState> GetBoxConstraint(const airtimeState &loc1, const airtimeState &loc2);

	/** Clear the constraints */
	void ClearStaticConstraints();

	/// STATE MANAGEMENT

	/** Get the possible actions from a state */
	virtual void GetActions(const airtimeState &nodeID, std::vector<airplaneAction> &actions) const;
	virtual void GetReverseActions(const airtimeState &nodeID, std::vector<airplaneAction> &actions) const;
	/** Get the successor states not violating constraints */
	virtual void GetSuccessors(const airtimeState &nodeID, std::vector<airtimeState> &neighbors) const;
	virtual void GetReverseSuccessors(const airtimeState &nodeID, std::vector<airtimeState> &neighbors) const;
	/** Apply an action to the base environment */
	virtual void ApplyAction(airtimeState &s, airplaneAction a) const;
	/** Undo an action on the base environment */
	virtual void UndoAction(airtimeState &s, airplaneAction a) const;
	/** Get the action required to move between two states */
	virtual airplaneAction GetAction(const airtimeState &node1, const airtimeState &node2) const;
	/** Given one state, get the next state */
	virtual void GetNextState(const airtimeState &currents, airplaneAction dir, airtimeState &news) const;
	/** Invert an action */
	virtual bool InvertAction(airplaneAction &a) const { return false; } // Actions are not invertible
	
	/// OCCUPANCY

	/** Deal with the occupancy */
	virtual OccupancyInterface<airtimeState,airplaneAction> *GetOccupancyInfo() { return 0; } //TODO: Not implemented
	
	/// HEURISTICS

	/** Heuristic value between two arbitrary nodes. **/
	virtual double HCost(const airtimeState &node1, const airtimeState &node2) const;
	virtual double HCost(const airtimeState &)  const { assert(false); return 0; } //No single state H-Cost implemented
	virtual double GCost(const airtimeState &node1, const airtimeState &node2) const {
	  float softCost(0);
	  for(auto const& sc: sconstraints){
	    softCost += sc.cost(node2,softConstraintEffectiveness);
	  }
	  return ae->GCost(node1,node2)+softCost;
	}
        virtual double GCost(const airtimeState &node, const airplaneAction &act) const {
          airtimeState node2(node);
          ApplyAction(node2,act);
          return GCost(node,node2);
        }
	virtual double GetPathLength(const std::vector<airtimeState> &n) const;
        virtual bool collisionCheck(const airtimeState &s1, const airtimeState &d1, float r1, const airtimeState &s2, const airtimeState &d2, float r2);

	/// GOAL TESTING

	virtual bool GoalTest(const airtimeState &) const { assert(false); return false; } // No support for single state goal testing
	virtual bool GoalTest(const airtimeState &node, const airtimeState &goal) const;

	/// HASHING
	
	/** Methods for dealing with state hashing */
	virtual uint64_t GetStateHash(const airtimeState &node) const;
        void GetStateFromHash(uint64_t hash, airtimeState &node) const;
	virtual uint64_t GetActionHash(airplaneAction act) const;

	/// DRAWING

	/** GL Drawing */
	virtual void OpenGLDraw() const;
	virtual void OpenGLDraw(const airtimeState&) const;
	virtual void OpenGLDraw(const airtimeState&, const airplaneAction&) const;
	virtual void GLDrawLine(const airtimeState &x, const airtimeState &y) const;
	virtual void OpenGLDraw(const airtimeState& oldState, const airtimeState &newState, float perc) const;
	virtual void GLDrawPath(const std::vector<airtimeState> &p,const std::vector<airtimeState> &wpts) const;
        virtual Map* GetMap()const{return nullptr;}

	// Override the color method.
	virtual void SetColor(double r, double g, double b, double t = 1.0) const {this->ae->SetColor(r,g,b,t);}
	virtual void SetColor(double& r, double& g, double& b, double& t) const {this->ae->SetColor(r,g,b,t);}

	/// UTILS
	uint8_t GetSpeeds(){return ae->numSpeeds;}
	//void SetTicketAuthority(TicketAuthority* tk) {this->ticket_authority = tk;}
	//TicketAuthority* ticket_authority;
	/** Vector holding the current constraints */
	std::vector<SoftConstraint<airtimeState>> sconstraints;

	AirplaneEnvironment *ae;
	void setSoftConstraintEffectiveness(float v){softConstraintEffectiveness=v;}
	float getSoftConstraintEffectiveness()const{return softConstraintEffectiveness;}
protected:
	const int width;
	const int length;
	const int height;
	float softConstraintEffectiveness=0.0;
private:
	


	std::vector<Collision<airtimeState>> static_constraints;

	/** Map holding the current sets of restricted airspace */


	/** Airplane Environment holder */
};

template <typename state, typename action>
class UnitTieBreaking {
  public:
  bool operator()(const AStarOpenClosedData<state> &ci1, const AStarOpenClosedData<state> &ci2) const
  {
    if (fequal(ci1.g+ci1.h, ci2.g+ci2.h)) // F-cost equal
    {
      
      if(useCAT && CAT){
        // Make them non-const :)
        AStarOpenClosedData<state>& i1(const_cast<AStarOpenClosedData<state>&>(ci1));
        AStarOpenClosedData<state>& i2(const_cast<AStarOpenClosedData<state>&>(ci2));

        // Compute cumulative conflicts (if not already done)
        if(i1.data.nc ==-1){
          //std::cout << "Getting NC for " << i1.data << ":\n";

          // Get number of conflicts in the parent
          state const*const parent1(i1.parentID?&(openList->Lookat(i1.parentID).data):nullptr);
          unsigned nc1(parent1?parent1->nc:0);
          //std::cout << "  matches " << matches.size() << "\n";

          // Count number of conflicts
          for(int agent(0); agent<CAT->numAgents(); ++agent){
            if(currentAgent == agent) continue;
            state const* p(0);
            if(i1.data.t!=0)
              p=&(CAT->get(agent,i1.data.t-1));
            state const& n=CAT->get(agent,i1.data.t);
            nc1+=checkForTheConflict(parent1,&i1.data,p,&n);
          }
          // Set the number of conflicts in the data object
          i1.data.nc=nc1;
        }
        if(i2.data.nc ==-1){
          //std::cout << "Getting NC for " << i2.data << ":\n";

          // Get number of conflicts in the parent
          state const*const parent2(i2.parentID?&(openList->Lookat(i2.parentID).data):nullptr);
          unsigned nc2(parent2?parent2->nc:0);
          //std::cout << "  matches " << matches.size() << "\n";

          // Count number of conflicts
          for(int agent(0); agent<CAT->numAgents(); ++agent){
            if(currentAgent == agent) continue;
            state const* p(0);
            if(i2.data.t!=0)
              p=&(CAT->get(agent,i2.data.t-1));
            state const& n=CAT->get(agent,i2.data.t);
            nc2+=checkForTheConflict(parent2,&i2.data,p,&n);
          }
          // Set the number of conflicts in the data object
          i2.data.nc=nc2;
        }
        if(fequal(i1.data.nc,i2.data.nc)){
          // Tie break towards states facing the goal
          unsigned facingGoal1(ci1.data.headingTo(currentEnv->getGoal()));
          unsigned facingGoal2(ci1.data.headingTo(currentEnv->getGoal()));
          unsigned hdiff1(Util::angleDiff<8>(ci1.data.heading,facingGoal1));
          unsigned hdiff2(Util::angleDiff<8>(ci2.data.heading,facingGoal2));
          if(hdiff1==hdiff2){
            if(randomalg && fequal(ci1.g,ci2.g)){
              return rand()%2;
            }
            return (fless(ci1.g, ci2.g));  // Tie-break toward greater g-cost
          }
          return fgreater(hdiff1,hdiff2);
        }
        return fgreater(i1.data.nc,i2.data.nc);
      }else{
          // Tie break towards states facing the goal
          unsigned facingGoal1(ci1.data.headingTo(currentEnv->getGoal()));
          unsigned facingGoal2(ci1.data.headingTo(currentEnv->getGoal()));
          unsigned hdiff1(Util::angleDiff<8>(ci1.data.heading,facingGoal1));
          unsigned hdiff2(Util::angleDiff<8>(ci2.data.heading,facingGoal2));
          if(hdiff1==hdiff2){
            if(randomalg && fequal(ci1.g,ci2.g)){
              return rand()%2;
            }
            return (fless(ci1.g, ci2.g));  // Tie-break toward greater g-cost
          }
          return fgreater(hdiff1,hdiff2);
        }
    }
    return (fgreater(ci1.g+ci1.h, ci2.g+ci2.h));
  }
    static OpenClosedInterface<state,AStarOpenClosedData<state>>* openList;
    static ConstrainedEnvironment<state,action>* currentEnv;
    static uint8_t currentAgent;
    static bool randomalg;
    static bool useCAT;
    static UnitTimeCAT<state,action>* CAT; // Conflict Avoidance Table
};

#define AIR_HASH_INTERVAL 0.09

template <typename state, typename action>
class RandomTieBreaking {
  public:
  bool operator()(const AStarOpenClosedData<state> &ci1, const AStarOpenClosedData<state> &ci2) const
  {
    if (fequal(ci1.g+ci1.h, ci2.g+ci2.h)) // F-cost equal
    {
      
      if(useCAT && CAT){
        // Make them non-const :)
        AStarOpenClosedData<state>& i1(const_cast<AStarOpenClosedData<state>&>(ci1));
        AStarOpenClosedData<state>& i2(const_cast<AStarOpenClosedData<state>&>(ci2));
        // Compute cumulative conflicts (if not already done)
        ConflictSet matches;
        if(i1.data.nc ==-1){
          //std::cout << "Getting NC for " << i1.data << ":\n";
          CAT->get(i1.data.t,i1.data.t+AIR_HASH_INTERVAL,matches);

          // Get number of conflicts in the parent
          state const*const parent1(i1.parentID?&(openList->Lookat(i1.parentID).data):nullptr);
          unsigned nc1(parent1?parent1->nc:0);
          //std::cout << "  matches " << matches.size() << "\n";

          // Count number of conflicts
          for(auto const& m: matches){
            if(currentAgent == m.agent) continue;
            state p;
            currentEnv->GetStateFromHash(m.hash1,p);
            //p.t=m.start;
            state n;
            currentEnv->GetStateFromHash(m.hash2,n);
            //n.t=m.stop;
            nc1+=checkForConflict(parent1,&i1.data,&p,&n);
            //if(!nc1){std::cout << "NO ";}
            //std::cout << "conflict(1): " << i1.data << " " << n << "\n";
          }
          // Set the number of conflicts in the data object
          i1.data.nc=nc1;
        }
        if(i2.data.nc ==-1){
          //std::cout << "Getting NC for " << i2.data << ":\n";
          CAT->get(i2.data.t,i2.data.t+AIR_HASH_INTERVAL,matches);

          // Get number of conflicts in the parent
          state const*const parent2(i2.parentID?&(openList->Lookat(i2.parentID).data):nullptr);
          unsigned nc2(parent2?parent2->nc:0);
          //std::cout << "  matches " << matches.size() << "\n";

          // Count number of conflicts
          for(auto const& m: matches){
            if(currentAgent == m.agent) continue;
            state p;
            currentEnv->GetStateFromHash(m.hash2,p);
            //p.t=m.start;
            state n;
            currentEnv->GetStateFromHash(m.hash2,n);
            //n.t=m.stop;
            nc2+=checkForConflict(parent2,&i2.data,&p,&n);
            //if(!nc2){std::cout << "NO ";}
            //std::cout << "conflict(2): " << i2.data << " " << n << "\n";
          }
          // Set the number of conflicts in the data object
          i2.data.nc=nc2;
        }
        if(fequal(i1.data.nc,i2.data.nc)){
          // Tie break towards states facing the goal
          unsigned facingGoal1(ci1.data.headingTo(currentEnv->getGoal()));
          unsigned facingGoal2(ci1.data.headingTo(currentEnv->getGoal()));
          unsigned hdiff1(Util::angleDiff<8>(ci1.data.heading,facingGoal1));
          unsigned hdiff2(Util::angleDiff<8>(ci2.data.heading,facingGoal2));
          if(hdiff1==hdiff2){
            if(randomalg && fequal(ci1.g,ci2.g)){
              return rand()%2;
            }
            return (fless(ci1.g, ci2.g));  // Tie-break toward greater g-cost
          }
          return fgreater(hdiff1,hdiff2);
        }
        return fgreater(i1.data.nc,i2.data.nc);
      }else{
          // Tie break towards states facing the goal
          unsigned facingGoal1(ci1.data.headingTo(currentEnv->getGoal()));
          unsigned facingGoal2(ci1.data.headingTo(currentEnv->getGoal()));
          unsigned hdiff1(Util::angleDiff<8>(ci1.data.heading,facingGoal1));
          unsigned hdiff2(Util::angleDiff<8>(ci2.data.heading,facingGoal2));
          if(hdiff1==hdiff2){
            if(randomalg && fequal(ci1.g,ci2.g)){
              return rand()%2;
            }
            return (fless(ci1.g, ci2.g));  // Tie-break toward greater g-cost
          }
          return fgreater(hdiff1,hdiff2);
        }
    }
    return (fgreater(ci1.g+ci1.h, ci2.g+ci2.h));
  }
    static OpenClosedInterface<state,AStarOpenClosedData<state>>* openList;
    static ConstrainedEnvironment<state,action>* currentEnv;
    static uint8_t currentAgent;
    static bool randomalg;
    static bool useCAT;
    static NonUnitTimeCAT<state,action>* CAT; // Conflict Avoidance Table
};

template <typename state, typename action>
OpenClosedInterface<state,AStarOpenClosedData<state>>* RandomTieBreaking<state,action>::openList=0;
template <typename state, typename action>
ConstrainedEnvironment<state,action>* RandomTieBreaking<state,action>::currentEnv=0;
template <typename state, typename action>
uint8_t RandomTieBreaking<state,action>::currentAgent=0;
template <typename state, typename action>
bool RandomTieBreaking<state,action>::randomalg=false;
template <typename state, typename action>
bool RandomTieBreaking<state,action>::useCAT=false;
template <typename state, typename action>
NonUnitTimeCAT<state,action>* RandomTieBreaking<state,action>::CAT=0;

#endif /* defined(__hog2_glut__AirplaneConstrainedEnvironment__) */
