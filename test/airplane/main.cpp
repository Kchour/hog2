#include "TestEnvironment.h"
#include "UnitTests.h"

int main(int argc, char **argv){
  //testAdmissibility();
  test2DPathUniqueness();

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();

  //testCardinalEnvHCost();
  //testSimpleActions();
  //testNormalActions();
  //testCardinalActions();
  //testHighway8Actions();
  //testCardinalHighwayActions();
  //testGridCardinalActions();
  //testGrid3DOctileActions();
  //testCardinalHeadingTo();
  //testMultiAgent();
  //TestQuadcopterActions();
  //testGetAction();
  //testLoadPerimeterHeuristic();
  //testHCost();
  //testHeadingTo();
  //testConstraints();
  //compareHeuristicSpeed();
  //testHash();
  //testIntervalTree();
  //testPEAStar();

  return 0;

}
