//
//  MultiAgent.cpp
//  hog2 glut
//
//  Created by Thayne Walker on 5/4/16.
//  Copyright © 2016 University of Denver. All rights reserved.
//

#include <stdio.h>
#include "MultiAgentEnvironment.h"
#include <iostream>

#if defined(__APPLE__)
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif

#include "TemplateAStar.h"
#include "Heuristic.h"

