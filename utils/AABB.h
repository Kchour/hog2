/*
  Copyright (c) 2009 Erin Catto http://www.box2d.org
  Copyright (c) 2016-2017 Lester Hedges <lester.hedges+aabbcc@gmail.com>
  Copyright (c) 2017 Thayne Walker

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.

  This code was adapted from parts of the Box2D Physics Engine,
  http://www.box2d.org
*/

#ifndef _AABB_H
#define _AABB_H

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <vector>

/// Null node flag.
const unsigned int NULL_NODE = 0xffffffff;

namespace aabb
{
    /*! \brief The axis-aligned bounding box object.

        Axis-aligned bounding boxes (AABBs) store information for the minimum
        orthorhombic bounding-box for an object in two- or three-dimensional
        space (the bounding box is either a rectangle, or rectangular prism).

        Class member functions provide functionality for merging AABB objects
        and testing overlap with other AABBs.
     */
    class AABB
    {
    public:
        /// Constructor.
        AABB();

        //! Constructor.
        /*! \param dimension
                The dimensionality of the system.
         */
        AABB(unsigned);

        //! Constructor.
        /*! \param lowerBound_
                The lower bound in each dimension.

            \param upperBound_
                The upper bound in each dimension.
         */
        AABB(const std::vector<double>&, const std::vector<double>&);

        /// Compute the surface area of the box.
        double computeSurfaceArea() const;

        /// Get the surface area of the box.
        double getSurfaceArea() const;

        //! Merge two AABBs into this one.
        /*! \param aabb1
                A reference to the first AABB.

            \param aabb2
                A reference to the second AABB.
         */
        void merge(const AABB&, const AABB&);

        //! Test whether the AABB is contained within this one.
        /*! \param aabb
                A reference to the AABB.

            \return
                Whether the AABB is fully contained.
         */
        bool contains(const AABB&) const;

        //! Test whether the AABB overlaps this one.
        /*! \param aabb
                A reference to the AABB.

            \return
                Whether the AABB overlaps.
         */
        bool overlaps(const AABB&) const;

        //! Compute the centre of the AABB.
        /*! \returns
                The position vector of the AABB centre.
         */
        std::vector<double> computeCentre();

        //! Set the dimensionality of the AABB.
        /*! \param dimension
                The dimensionality of the system.
         */
        void setDimension(unsigned int);

        /// Lower bound of AABB in each dimension.
        std::vector<double> lowerBound;

        /// Upper bound of AABB in each dimension.
        std::vector<double> upperBound;

        /// The position of the AABB centre.
        std::vector<double> centre;

        /// The AABB's surface area.
        double surfaceArea;
    };

    /*! \brief A node of the AABB tree.

        Each node of the tree contains an AABB object which corresponds to a
        particle, or a group of particles, in the simulation box.

        Nodes are aware of their position within in the tree. The isLeaf member
        function allows the tree to query whether the node is a leaf, i.e. to
        determine whether it holds a single particle.
     */
    struct Node
    {
        /// Constructor.
        Node();

        /// The fattened axis-aligned bounding box.
        AABB aabb;

        /// Index of the parent node.
        unsigned int parent;

        /// Index of the next node.
        unsigned int next;

        /// Index of the left-hand child.
        unsigned int left;

        /// Index of the right-hand child.
        unsigned int right;

        /// Height of the node. This is 0 for a leaf and -1 for a free node.
        int height;

        /// The index of the particle that the node contains (leaf nodes only).
        unsigned int particle;

        //! Test whether the node is a leaf.
        /*! \return
                Whether the node is a leaf node.
         */
        bool isLeaf() const;
    };

    /*! \brief The dynamic AABB tree.

        The dynamic AABB tree is a hierarchical data structure that can be used
        to efficiently query overlaps between objects of arbitrary shape and
        size that lie inside of a simulation box. Support is provided for
        periodic and non-periodic boxes, as well as boxes with partial
        periodicity, e.g. periodic along specific axes.
     */
    class Tree
    {
    public:
        //! Constructor (non-periodic).
        /*! \param dimension_
                The dimensionality of the system.

            \param nParticles
                The number of particles (for fixed particle number systems).
         */
        Tree(unsigned int dimension_= 3, unsigned int nParticles = 16);

        //! Constructor (custom periodicity).
        /*! \param dimension_
                The dimensionality of the system.

            \param periodicity_
                Whether the system is periodic in each dimension.

            \param boxSize_
                The size of the simulation box in each dimension.

            \param nParticles
                The number of particles (for fixed particle number systems).
         */
        Tree(unsigned int, const std::vector<bool>&, const std::vector<double>&, unsigned int nParticles = 16);

        //! Set the periodicity of the simulation box.
        /*! \param periodicity_
                Whether the system is periodic in each dimension.
         */
        void setPeriodicity(const std::vector<bool>&);

        //! Set the size of the simulation box.
        /*! \param boxSize_
                The size of the simulation box in each dimension.
         */
        void setBoxSize(const std::vector<double>&);

        //! Insert a particle into the tree (point particle).
        /*! \param index
                The index of the particle.

            \param position
                The position vector of the particle.

            \param radius
                The radius of the particle.
         */
        void insertParticle(unsigned int, std::vector<double>&, double);

        //! Insert a particle into the tree (arbitrary shape with bounding box).
        /*! \param index
                The index of the particle.

            \param lowerBound
                The lower bound in each dimension.

            \param upperBound
                The upper bound in each dimension.
         */
        void insertParticle(unsigned int, std::vector<double>&, std::vector<double>&);

        //! Remove a particle from the tree.
        /*! \param particle
                The particle index (particleMap will be used to map the node).
         */
        void removeParticle(unsigned int);

        //! Update the tree if a particle moves outside its fattened AABB.
        /*! \param particle
                The particle index (particleMap will be used to map the node).

            \param position
                The position vector of the particle.

            \param radius
                The radius of the particle.

            \return
                Whether the particle was reinserted.
         */
        bool updateParticle(unsigned int, std::vector<double>&, double);

        //! Update the tree if a particle moves outside its fattened AABB.
        /*! \param particle
                The particle index (particleMap will be used to map the node).

            \param lowerBound
                The lower bound in each dimension.

            \param upperBound
                The upper bound in each dimension.

         */
        bool updateParticle(unsigned int, std::vector<double>&, std::vector<double>&);

        //! Query the tree to find candidate interactions for a particle.
        /*! \param particle
                The particle index.

            \return particles
                A vector of particle indices.
         */
        std::vector<unsigned int> query(unsigned int);

        //! Query the tree to find candidate interactions for an AABB.
        /*! \param particle
                The particle index.

            \param aabb
                The AABB.

            \return particles
                A vector of particle indices.
         */
        std::vector<unsigned int> query(unsigned int, const AABB&);

        //! Query the tree to find candidate interactions for an AABB.
        /*! \param aabb
                The AABB.

            \return particles
                A vector of particle indices.
         */
        std::vector<unsigned int> query(const AABB&);

        //! Get a particle AABB.
        /*! \param particle
                The particle index.
         */
        const AABB& getAABB(unsigned int);

        //! Get the height of the tree.
        /*! \return
                The height of the binary tree.
         */
        unsigned int getHeight() const;

        //! Get the number of nodes in the tree.
        /*! \return
                The number of nodes in the tree.
         */
        unsigned int getNodeCount() const;

        //! Compute the maximum balancance of the tree.
        /*! \return
                The maximum difference between the height of two
                children of a node.
         */
        unsigned int computeMaximumBalance() const;

        //! Compute the surface area ratio of the tree.
        /*! \return
                The ratio of the sum of the node surface area to the surface
                area of the root node.
         */
        double computeSurfaceAreaRatio() const;

        /// Validate the tree.
        void validate() const;

        /// Rebuild an optimal tree.
        void rebuild();

    private:
        /// The index of the root node.
        unsigned int root;

        /// The dynamic tree.
        std::vector<Node> nodes;

        /// The current number of nodes in the tree.
        unsigned int nodeCount;

        /// The current node capacity.
        unsigned int nodeCapacity;

        /// The position of node at the top of the free list.
        unsigned int freeList;

        /// The dimensionality of the system.
        unsigned int dimension;

        /// Whether the system is periodic along at least one axis.
        bool isPeriodic;

        /// Whether the system is periodic along each axis.
        std::vector<bool> periodicity;

        /// The size of the system in each dimension.
        std::vector<double> boxSize;

        /// The position of the negative minimum image.
        std::vector<double> negMinImage;

        /// The position of the positive minimum image.
        std::vector<double> posMinImage;

        /// A map between particle and node indices.
        std::map<unsigned int, unsigned int> particleMap;

        //! Allocate a new node.
        /*! \return
                The index of the allocated node.
         */
        unsigned int allocateNode();

        //! Free an existing node.
        /*! \param node
                The index of the node to be freed.
         */
        void freeNode(unsigned int);

        //! Insert a leaf into the tree.
        /*! \param leaf
                The index of the leaf node.
         */
        void insertLeaf(unsigned int);

        //! Remove a leaf from the tree.
        /*! \param leaf
                The index of the leaf node.
         */
        void removeLeaf(unsigned int);

        //! Balance the tree.
        /*! \param node
                The index of the node.
         */
        unsigned int balance(unsigned int);

        //! Compute the height of the tree.
        /*! \return
                The height of the entire tree.
         */
        unsigned int computeHeight() const;

        //! Compute the height of a sub-tree.
        /*! \param node
                The index of the root node.

            \return
                The height of the sub-tree.
         */
        unsigned int computeHeight(unsigned int) const;

        //! Assert that the sub-tree has a valid structure.
        /*! \param node
                The index of the root node.
         */
        void validateStructure(unsigned int) const;

        //! Assert that the sub-tree has valid metrics.
        /*! \param node
                The index of the root node.
         */
        void validateMetrics(unsigned int) const;

        //! Apply periodic boundary conditions.
        /* \param position
                The position vector.
         */
        void periodicBoundaries(std::vector<double>&);

        //! Compute minimum image separation.
        /*! \param separation
                The separation vector.

            \param shift
                The shift vector.

            \return
                Whether a periodic shift has been applied.
         */
        bool minimumImage(std::vector<double>&, std::vector<double>&);
    };

    AABB::AABB()
    {
    }

    AABB::AABB(unsigned int dimension)
    {
        assert((dimension == 2) || (dimension == 3));

        lowerBound.resize(dimension);
        upperBound.resize(dimension);
    }

    AABB::AABB(const std::vector<double>& lowerBound_, const std::vector<double>& upperBound_) :
        lowerBound(lowerBound_), upperBound(upperBound_)
    {
        surfaceArea = computeSurfaceArea();
        centre = computeCentre();
    }

    double AABB::computeSurfaceArea() const
    {
        // Calculate the perimeter of the 2D AABB.
        if (lowerBound.size() == 2)
        {
            double wx = upperBound[0] - lowerBound[0];
            double wy = upperBound[1] - lowerBound[1];
            return 2.0 * (wx + wy);
        }

        // Calculate the surface area of the 3D AABB.
        else
        {
            double wx = upperBound[0] - lowerBound[0];
            double wy = upperBound[1] - lowerBound[1];
            double wz = upperBound[2] - lowerBound[2];
            return 2.0 * (wx*wy + wx*wz + wy*wz);
        }
    }

    double AABB::getSurfaceArea() const
    {
        return surfaceArea;
    }

    void AABB::merge(const AABB& aabb1, const AABB& aabb2)
    {
        assert(aabb1.lowerBound.size() == aabb2.lowerBound.size());
        assert(aabb1.upperBound.size() == aabb2.upperBound.size());

        lowerBound.resize(aabb1.lowerBound.size());
        upperBound.resize(aabb1.lowerBound.size());

        for (unsigned int i=0;i<lowerBound.size();i++)
        {
            lowerBound[i] = std::min(aabb1.lowerBound[i], aabb2.lowerBound[i]);
            upperBound[i] = std::max(aabb1.upperBound[i], aabb2.upperBound[i]);
        }

        surfaceArea = computeSurfaceArea();
        centre = computeCentre();
    }

    bool AABB::contains(const AABB& aabb) const
    {
        assert(aabb.lowerBound.size() == lowerBound.size());

        for (unsigned int i=0;i<lowerBound.size();i++)
        {
            if (lowerBound[i] < aabb.lowerBound[i]) return false;
            if (upperBound[i] > aabb.upperBound[i]) return false;
        }

        return true;
    }

    bool AABB::overlaps(const AABB& aabb) const
    {
        assert(aabb.lowerBound.size() == lowerBound.size());

        if (lowerBound.size() == 2)
        {
            return !(   aabb.upperBound[0] < lowerBound[0]
                     || aabb.lowerBound[0] > upperBound[0]
                     || aabb.upperBound[1] < lowerBound[1]
                     || aabb.lowerBound[1] > upperBound[1]
                    );
        }
        else
        {
            return !(   aabb.upperBound[0] < lowerBound[0]
                     || aabb.lowerBound[0] > upperBound[0]
                     || aabb.upperBound[1] < lowerBound[1]
                     || aabb.lowerBound[1] > upperBound[1]
                     || aabb.upperBound[2] < lowerBound[2]
                     || aabb.lowerBound[2] > upperBound[2]
                    );
        }
    }

    std::vector<double> AABB::computeCentre()
    {
        std::vector<double> position(lowerBound.size());

        for (unsigned int i=0;i<position.size();i++)
            position[i] = 0.5 * (lowerBound[i] + upperBound[i]);

        return position;
    }

    void AABB::setDimension(unsigned int dimension)
    {
        assert((dimension == 2) || (dimension == 3));

        lowerBound.resize(dimension);
        upperBound.resize(dimension);
    }

    Node::Node()
    {
    }

    bool Node::isLeaf() const
    {
        return (left == NULL_NODE);
    }

    Tree::Tree(unsigned int dimension_,
               unsigned int nParticles) :
        dimension(dimension_), isPeriodic(false)
    {
        // Validate the dimensionality.
        if ((dimension != 2) && (dimension != 3))
        {
            std::cerr << "[ERROR]: Invalid dimensionality!" << '\n';
            exit(EXIT_FAILURE);
        }

        // Initialise the periodicity vector.
        periodicity.resize(dimension);
        std::fill(periodicity.begin(), periodicity.end(), false);

        // Initialise the tree.
        root = NULL_NODE;
        nodeCount = 0;
        nodeCapacity = nParticles;
        nodes.resize(nodeCapacity);

        // Build a linked list for the list of free nodes.
        for (unsigned int i=0;i<nodeCapacity-1;i++)
        {
            nodes[i].next = i + 1;
            nodes[i].height = -1;
        }
        nodes[nodeCapacity-1].next = NULL_NODE;
        nodes[nodeCapacity-1].height = -1;

        // Assign the index of the first free node.
        freeList = 0;
    }

    Tree::Tree(unsigned int dimension_,
               const std::vector<bool>& periodicity_,
               const std::vector<double>& boxSize_,
               unsigned int nParticles) :
        dimension(dimension_), periodicity(periodicity_), boxSize(boxSize_)
    {
        // Validate the dimensionality.
        if ((dimension != 2) && (dimension != 3))
        {
            std::cerr << "[ERROR]: Invalid dimensionality!" << '\n';
            exit(EXIT_FAILURE);
        }

        // Validate the dimensionality of the vectors.
        if ((periodicity.size() != dimension) || (boxSize.size() != dimension))
        {
            std::cerr << "[ERROR]: Dimensionality mismatch!" << '\n';
            exit(EXIT_FAILURE);
        }

        // Initialise the tree.
        root = NULL_NODE;
        nodeCount = 0;
        nodeCapacity = nParticles;
        nodes.resize(nodeCapacity);

        // Build a linked list for the list of free nodes.
        for (unsigned int i=0;i<nodeCapacity-1;i++)
        {
            nodes[i].next = i + 1;
            nodes[i].height = -1;
        }
        nodes[nodeCapacity-1].next = NULL_NODE;
        nodes[nodeCapacity-1].height = -1;

        // Assign the index of the first free node.
        freeList = 0;

        // Check periodicity.
        isPeriodic = false;
        posMinImage.resize(dimension);
        negMinImage.resize(dimension);
        for (unsigned int i=0;i<dimension;i++)
        {
            posMinImage[i] =  0.5*boxSize[i];
            negMinImage[i] = -0.5*boxSize[i];

            if (periodicity[i])
                isPeriodic = true;
        }
    }

    void Tree::setPeriodicity(const std::vector<bool>& periodicity_)
    {
        periodicity = periodicity_;
    }

    void Tree::setBoxSize(const std::vector<double>& boxSize_)
    {
        boxSize = boxSize_;
    }

    unsigned int Tree::allocateNode()
    {
        // Expand the node pool as needed.
        if (freeList == NULL_NODE)
        {
            assert(nodeCount == nodeCapacity);

            // The free list is empty. Rebuild a bigger pool.
            nodeCapacity *= 2;
            nodes.resize(nodeCapacity);

            // Build a linked list for the list of free nodes.
            for (unsigned int i=nodeCount;i<nodeCapacity-1;i++)
            {
                nodes[i].next = i + 1;
                nodes[i].height = -1;
            }
            nodes[nodeCapacity-1].next = NULL_NODE;
            nodes[nodeCapacity-1].height = -1;

            // Assign the index of the first free node.
            freeList = nodeCount;
        }

        // Peel a node off the free list.
        unsigned int node = freeList;
        freeList = nodes[node].next;
        nodes[node].parent = NULL_NODE;
        nodes[node].left = NULL_NODE;
        nodes[node].right = NULL_NODE;
        nodes[node].height = 0;
        nodes[node].aabb.setDimension(dimension);
        nodeCount++;

        return node;
    }

    void Tree::freeNode(unsigned int node)
    {
        assert(0 <= node && node < nodeCapacity);
        assert(0 < nodeCount);

        nodes[node].next = freeList;
        nodes[node].height = -1;
        freeList = node;
        nodeCount--;
    }

    void Tree::insertParticle(unsigned int particle, std::vector<double>& position, double radius)
    {
        // Validate the dimensionality of the position vector.
        if (position.size() != dimension)
        {
            std::cerr << "[ERROR]: Dimensionality mismatch!" << '\n';
            exit(EXIT_FAILURE);
        }

        // Allocate a new node for the particle.
        unsigned int node = allocateNode();

        // Compute the AABB limits.
        for (unsigned i=0;i<dimension;i++)
        {
            nodes[node].aabb.lowerBound[i] = position[i] - radius;
            nodes[node].aabb.upperBound[i] = position[i] + radius;
        }

        nodes[node].aabb.surfaceArea = nodes[node].aabb.computeSurfaceArea();
        nodes[node].aabb.centre = nodes[node].aabb.computeCentre();

        // Zero the height.
        nodes[node].height = 0;

        // Insert a new leaf into the tree.
        insertLeaf(node);

        // Add the new particle to the map.
        particleMap.insert(std::map<unsigned int, unsigned int>::value_type(particle, node));

        // Store the particle index.
        nodes[node].particle = particle;
    }

    void Tree::insertParticle(unsigned int particle, std::vector<double>& lowerBound, std::vector<double>& upperBound)
    {
        // Validate the dimensionality of the bounds vectors.
        if ((lowerBound.size() != dimension) || (upperBound.size() != dimension))
        {
            std::cerr << "[ERROR]: Dimensionality mismatch!" << '\n';
            exit(EXIT_FAILURE);
        }

        // Allocate a new node for the particle.
        unsigned int node = allocateNode();

        // Compute the AABB limits.
        for (unsigned i=0;i<dimension;i++)
        {
            // Validate the bound.
            if (lowerBound[i] >= upperBound[i])
            {
                std::cerr << "[ERROR]: AABB lower bound is greater than the upper bound!" << '\n';
                exit(EXIT_FAILURE);
            }

            nodes[node].aabb.lowerBound[i] = lowerBound[i];
            nodes[node].aabb.upperBound[i] = upperBound[i];
        }

        nodes[node].aabb.surfaceArea = nodes[node].aabb.computeSurfaceArea();
        nodes[node].aabb.centre = nodes[node].aabb.computeCentre();

        // Zero the height.
        nodes[node].height = 0;

        // Insert a new leaf into the tree.
        insertLeaf(node);

        // Add the new particle to the map.
        particleMap.insert(std::map<unsigned int, unsigned int>::value_type(particle, node));

        // Store the particle index.
        nodes[node].particle = particle;
    }

    void Tree::removeParticle(unsigned int particle)
    {
        // Map iterator.
        std::map<unsigned int, unsigned int>::iterator it;

        // Find the particle.
        it = particleMap.find(particle);

        // Extract the node index.
        unsigned int node = it->second;

        // Erase the particle from the map.
        particleMap.erase(it);

        assert(0 <= node && node < nodeCapacity);
        assert(nodes[node].isLeaf());

        removeLeaf(node);
        freeNode(node);
    }

    bool Tree::updateParticle(unsigned int particle, std::vector<double>& position, double radius)
    {
        // Validate the dimensionality of the position vector.
        if (position.size() != dimension)
        {
            std::cerr << "[ERROR]: Dimensionality mismatch!" << '\n';
            exit(EXIT_FAILURE);
        }

        // Extract the node index.
        unsigned int node = particleMap.find(particle)->second;

        assert(0 <= node && node < nodeCapacity);
        assert(nodes[node].isLeaf());

        // AABB bounds vectors.
        std::vector<double> lowerBound(dimension);
        std::vector<double> upperBound(dimension);

        // AABB size in each dimension.
        double size[dimension];

        // Compute the AABB limits.
        for (unsigned i=0;i<dimension;i++)
        {
            lowerBound[i] = position[i] - radius;
            upperBound[i] = position[i] + radius;
            size[i] = upperBound[i] - lowerBound[i];
        }

        // Create the new AABB.
        AABB aabb(lowerBound, upperBound);

        // No need to update if the particle is still within its fattened AABB.
        //if (nodes[node].aabb.contains(aabb)) return false;

        // Remove the current leaf.
        removeLeaf(node);

        // Assign the new AABB.
        nodes[node].aabb = aabb;

        // Update the surface area and centroid.
        nodes[node].aabb.surfaceArea = nodes[node].aabb.computeSurfaceArea();
        nodes[node].aabb.centre = nodes[node].aabb.computeCentre();

        // Insert a new leaf node.
        insertLeaf(node);

        return true;
    }

    bool Tree::updateParticle(unsigned int particle, std::vector<double>& lowerBound, std::vector<double>& upperBound)
    {
        // Validate the dimensionality of the bounds vectors.
        if ((lowerBound.size() != dimension) || (upperBound.size() != dimension))
        {
            std::cerr << "[ERROR]: Dimensionality mismatch!" << '\n';
            exit(EXIT_FAILURE);
        }

        // Extract the node index.
        unsigned int node = particleMap.find(particle)->second;

        assert(0 <= node && node < nodeCapacity);
        assert(nodes[node].isLeaf());

        // Compute the AABB limits.
        for (unsigned i=0;i<dimension;i++)
        {
            // Validate the bound.
            if (lowerBound[i] >= upperBound[i])
            {
                std::cerr << "[ERROR]: AABB lower bound is greater than the upper bound!" << '\n';
                exit(EXIT_FAILURE);
            }
        }

        // Create the new AABB.
        AABB aabb(lowerBound, upperBound);

        // No need to update if the particle is still within its fattened AABB.
        //if (nodes[node].aabb.contains(aabb)) return false;

        // Remove the current leaf.
        removeLeaf(node);

        // Assign the new AABB.
        nodes[node].aabb = aabb;

        // Update the surface area and centroid.
        nodes[node].aabb.surfaceArea = nodes[node].aabb.computeSurfaceArea();
        nodes[node].aabb.centre = nodes[node].aabb.computeCentre();

        // Insert a new leaf node.
        insertLeaf(node);

        return true;
    }

    std::vector<unsigned int> Tree::query(unsigned int particle)
    {
        assert(particleMap.count(particle));

        // Test overlap of particle AABB against all other particles.
        return query(particle, nodes[particleMap.find(particle)->second].aabb);
    }

    std::vector<unsigned int> Tree::query(unsigned int particle, const AABB& aabb)
    {
        std::vector<unsigned int> stack;
        stack.reserve(256);
        stack.push_back(root);

        std::vector<unsigned int> particles;

        while (stack.size() > 0)
        {
            unsigned int node = stack.back();
            stack.pop_back();

            // Copy the AABB.
            AABB nodeAABB = nodes[node].aabb;

            if (node == NULL_NODE) continue;

            if (isPeriodic)
            {
                std::vector<double> separation(dimension);
                std::vector<double> shift(dimension);
                for (unsigned int i=0;i<dimension;i++)
                    separation[i] = nodeAABB.centre[i] - aabb.centre[i];

                bool isShifted = minimumImage(separation, shift);

                // Shift the AABB.
                if (isShifted)
                {
                    for (unsigned int i=0;i<dimension;i++)
                    {
                        nodeAABB.lowerBound[i] += shift[i];
                        nodeAABB.upperBound[i] += shift[i];
                    }
                }
            }

            // Test for overlap between the AABBs.
            if (aabb.overlaps(nodeAABB))
            {
                // Check that we're at a leaf node.
                if (nodes[node].isLeaf())
                {
                    // Can't interact with itself.
                    if (nodes[node].particle != particle)
                        particles.push_back(nodes[node].particle);
                }
                else
                {
                    stack.push_back(nodes[node].left);
                    stack.push_back(nodes[node].right);
                }
            }
        }

        return particles;
    }

    std::vector<unsigned int> Tree::query(const AABB& aabb)
    {
        // Test overlap of AABB against all particles.
        return query(std::numeric_limits<unsigned int>::max(), aabb);
    }

    const AABB& Tree::getAABB(unsigned int particle)
    {
        return nodes[particleMap[particle]].aabb;
    }

    void Tree::insertLeaf(unsigned int leaf)
    {
        if (root == NULL_NODE)
        {
            root = leaf;
            nodes[root].parent = NULL_NODE;
            return;
        }

        // Find the best sibling for the node.

        AABB leafAABB = nodes[leaf].aabb;
        unsigned int index = root;

        while (!nodes[index].isLeaf())
        {
            // Extract the children of the node.
            unsigned int left  = nodes[index].left;
            unsigned int right = nodes[index].right;

            double surfaceArea = nodes[index].aabb.getSurfaceArea();

            AABB combinedAABB;
            combinedAABB.merge(nodes[index].aabb, leafAABB);
            double combinedSurfaceArea = combinedAABB.getSurfaceArea();

            // Cost of creating a new parent for this node and the new leaf.
            double cost = 2.0 * combinedSurfaceArea;

            // Minimum cost of pushing the leaf further down the tree.
            double inheritanceCost = 2.0 * (combinedSurfaceArea - surfaceArea);

            // Cost of descending to the left.
            double costLeft;
            if (nodes[left].isLeaf())
            {
                AABB aabb;
                aabb.merge(leafAABB, nodes[left].aabb);
                costLeft = aabb.getSurfaceArea() + inheritanceCost;
            }
            else
            {
                AABB aabb;
                aabb.merge(leafAABB, nodes[left].aabb);
                double oldArea = nodes[left].aabb.getSurfaceArea();
                double newArea = aabb.getSurfaceArea();
                costLeft = (newArea - oldArea) + inheritanceCost;
            }

            // Cost of descending to the right.
            double costRight;
            if (nodes[right].isLeaf())
            {
                AABB aabb;
                aabb.merge(leafAABB, nodes[right].aabb);
                costRight = aabb.getSurfaceArea() + inheritanceCost;
            }
            else
            {
                AABB aabb;
                aabb.merge(leafAABB, nodes[right].aabb);
                double oldArea = nodes[right].aabb.getSurfaceArea();
                double newArea = aabb.getSurfaceArea();
                costRight = (newArea - oldArea) + inheritanceCost;
            }

            // Descend according to the minimum cost.
            if ((cost < costLeft) && (cost < costRight)) break;

            // Descend.
            if (costLeft < costRight) index = left;
            else                      index = right;
        }

        unsigned int sibling = index;

        // Create a new parent.
        unsigned int oldParent = nodes[sibling].parent;
        unsigned int newParent = allocateNode();
        nodes[newParent].parent = oldParent;
        nodes[newParent].aabb.merge(leafAABB, nodes[sibling].aabb);
        nodes[newParent].height = nodes[sibling].height + 1;

        // The sibling was not the root.
        if (oldParent != NULL_NODE)
        {
            if (nodes[oldParent].left == sibling) nodes[oldParent].left = newParent;
            else                                  nodes[oldParent].right = newParent;

            nodes[newParent].left = sibling;
            nodes[newParent].right = leaf;
            nodes[sibling].parent = newParent;
            nodes[leaf].parent = newParent;
        }
        // The sibling was the root.
        else
        {
            nodes[newParent].left = sibling;
            nodes[newParent].right = leaf;
            nodes[sibling].parent = newParent;
            nodes[leaf].parent = newParent;
            root = newParent;
        }

        // Walk back up the tree fixing heights and AABBs.
        index = nodes[leaf].parent;
        while (index != NULL_NODE)
        {
            index = balance(index);

            unsigned int left = nodes[index].left;
            unsigned int right = nodes[index].right;

            assert(left != NULL_NODE);
            assert(right != NULL_NODE);

            nodes[index].height = 1 + std::max(nodes[left].height, nodes[right].height);
            nodes[index].aabb.merge(nodes[left].aabb, nodes[right].aabb);

            index = nodes[index].parent;
        }
    }

    void Tree::removeLeaf(unsigned int leaf)
    {
        if (leaf == root)
        {
            root = NULL_NODE;
            return;
        }

        unsigned int parent = nodes[leaf].parent;
        unsigned int grandParent = nodes[parent].parent;
        unsigned int sibling;

        if (nodes[parent].left == leaf) sibling = nodes[parent].right;
        else                            sibling = nodes[parent].left;

        // Destroy the parent and connect the sibling to the grandparent.
        if (grandParent != NULL_NODE)
        {
            if (nodes[grandParent].left == parent) nodes[grandParent].left = sibling;
            else                                   nodes[grandParent].right = sibling;

            nodes[sibling].parent = grandParent;
            freeNode(parent);

            // Adjust ancestor bounds.
            unsigned int index = grandParent;
            while (index != NULL_NODE)
            {
                index = balance(index);

                unsigned int left = nodes[index].left;
                unsigned int right = nodes[index].right;

                nodes[index].aabb.merge(nodes[left].aabb, nodes[right].aabb);
                nodes[index].height = 1 + std::max(nodes[left].height, nodes[right].height);

                index = nodes[index].parent;
            }
        }
        else
        {
            root = sibling;
            nodes[sibling].parent = NULL_NODE;
            freeNode(parent);
        }
    }

    unsigned int Tree::balance(unsigned int node)
    {
        assert(node != NULL_NODE);

        if (nodes[node].isLeaf() || (nodes[node].height < 2))
            return node;

        unsigned int left = nodes[node].left;
        unsigned int right = nodes[node].right;

        assert((0 <= left) && (left < nodeCapacity));
        assert((0 <= right) && (right < nodeCapacity));

        int currentBalance = nodes[right].height - nodes[left].height;

        // Rotate right branch up.
        if (currentBalance > 1)
        {
            unsigned int rightLeft = nodes[right].left;
            unsigned int rightRight = nodes[right].right;

            assert((0 <= rightLeft) && (rightLeft < nodeCapacity));
            assert((0 <= rightRight) && (rightRight < nodeCapacity));

            // Swap node and its right-hand child.
            nodes[right].left = node;
            nodes[right].parent = nodes[node].parent;
            nodes[node].parent = right;

            // The node's old parent should now point to its right-hand child.
            if (nodes[right].parent != NULL_NODE)
            {
                if (nodes[nodes[right].parent].left == node) nodes[nodes[right].parent].left = right;
                else
                {
                    assert(nodes[nodes[right].parent].right == node);
                    nodes[nodes[right].parent].right = right;
                }
            }
            else root = right;

            // Rotate.
            if (nodes[rightLeft].height > nodes[rightRight].height)
            {
                nodes[right].right = rightLeft;
                nodes[node].right = rightRight;
                nodes[rightRight].parent = node;
                nodes[node].aabb.merge(nodes[left].aabb, nodes[rightRight].aabb);
                nodes[right].aabb.merge(nodes[node].aabb, nodes[rightLeft].aabb);

                nodes[node].height = 1 + std::max(nodes[left].height, nodes[rightRight].height);
                nodes[right].height = 1 + std::max(nodes[node].height, nodes[rightLeft].height);
            }
            else
            {
                nodes[right].right = rightRight;
                nodes[node].right = rightLeft;
                nodes[rightLeft].parent = node;
                nodes[node].aabb.merge(nodes[left].aabb, nodes[rightLeft].aabb);
                nodes[right].aabb.merge(nodes[node].aabb, nodes[rightRight].aabb);

                nodes[node].height = 1 + std::max(nodes[left].height, nodes[rightLeft].height);
                nodes[right].height = 1 + std::max(nodes[node].height, nodes[rightRight].height);
            }

            return right;
        }

        // Rotate left branch up.
        if (currentBalance < -1)
        {
            unsigned int leftLeft = nodes[left].left;
            unsigned int leftRight = nodes[left].right;

            assert((0 <= leftLeft) && (leftLeft < nodeCapacity));
            assert((0 <= leftRight) && (leftRight < nodeCapacity));

            // Swap node and its left-hand child.
            nodes[left].left = node;
            nodes[left].parent = nodes[node].parent;
            nodes[node].parent = left;

            // The node's old parent should now point to its left-hand child.
            if (nodes[left].parent != NULL_NODE)
            {
                if (nodes[nodes[left].parent].left == node) nodes[nodes[left].parent].left = left;
                else
                {
                    assert(nodes[nodes[left].parent].right == node);
                    nodes[nodes[left].parent].right = left;
                }
            }
            else root = left;

            // Rotate.
            if (nodes[leftLeft].height > nodes[leftRight].height)
            {
                nodes[left].right = leftLeft;
                nodes[node].left = leftRight;
                nodes[leftRight].parent = node;
                nodes[node].aabb.merge(nodes[right].aabb, nodes[leftRight].aabb);
                nodes[left].aabb.merge(nodes[node].aabb, nodes[leftLeft].aabb);

                nodes[node].height = 1 + std::max(nodes[right].height, nodes[leftRight].height);
                nodes[left].height = 1 + std::max(nodes[node].height, nodes[leftLeft].height);
            }
            else
            {
                nodes[left].right = leftRight;
                nodes[node].left = leftLeft;
                nodes[leftLeft].parent = node;
                nodes[node].aabb.merge(nodes[right].aabb, nodes[leftLeft].aabb);
                nodes[left].aabb.merge(nodes[node].aabb, nodes[leftRight].aabb);

                nodes[node].height = 1 + std::max(nodes[right].height, nodes[leftLeft].height);
                nodes[left].height = 1 + std::max(nodes[node].height, nodes[leftRight].height);
            }

            return left;
        }

        return node;
    }

    unsigned int Tree::computeHeight() const
    {
        return computeHeight(root);
    }

    unsigned int Tree::computeHeight(unsigned int node) const
    {
        assert((0 <= node) && (node < nodeCapacity));

        if (nodes[node].isLeaf()) return 0;

        unsigned int height1 = computeHeight(nodes[node].left);
        unsigned int height2 = computeHeight(nodes[node].right);

        return 1 + std::max(height1, height2);
    }

    unsigned int Tree::getHeight() const
    {
        if (root == NULL_NODE) return 0;
        return nodes[root].height;
    }

    unsigned int Tree::getNodeCount() const
    {
        return nodeCount;
    }

    unsigned int Tree::computeMaximumBalance() const
    {
        unsigned int maxBalance = 0;
        for (unsigned int i=0; i<nodeCapacity; i++)
        {
            if (nodes[i].height <= 1)
                continue;

            assert(nodes[i].isLeaf() == false);

            unsigned int balance = std::abs(nodes[nodes[i].left].height - nodes[nodes[i].right].height);
            maxBalance = std::max(maxBalance, balance);
        }

        return maxBalance;
    }

    double Tree::computeSurfaceAreaRatio() const
    {
        if (root == NULL_NODE) return 0.0;

        double rootArea = nodes[root].aabb.computeSurfaceArea();
        double totalArea = 0.0;

        for (unsigned int i=0; i<nodeCapacity;i++)
        {
            if (nodes[i].height < 0) continue;

            totalArea += nodes[i].aabb.computeSurfaceArea();
        }

        return totalArea / rootArea;
    }

    void Tree::validate() const
    {
#ifndef NDEBUG
        validateStructure(root);
        validateMetrics(root);

        unsigned int freeCount = 0;
        unsigned int freeIndex = freeList;

        while (freeIndex != NULL_NODE)
        {
            assert((0 <= freeIndex) && (freeIndex < nodeCapacity));
            freeIndex = nodes[freeIndex].next;
            freeCount++;
        }

        assert(getHeight() == computeHeight());
        assert((nodeCount + freeCount) == nodeCapacity);
#endif
    }

    void Tree::rebuild()
    {
        unsigned int nodeIndices[nodeCount];
        unsigned int count = 0;

        for (unsigned int i=0;i<nodeCapacity;i++)
        {
            // Free node.
            if (nodes[i].height < 0) continue;

            if (nodes[i].isLeaf())
            {
                nodes[i].parent = NULL_NODE;
                nodeIndices[count] = i;
                count++;
            }
            else freeNode(i);
        }

        while (count > 1)
        {
            double minCost = std::numeric_limits<double>::max();
            int iMin = -1, jMin = -1;

            for (unsigned int i=0;i<count;i++)
            {
                AABB aabbi = nodes[nodeIndices[i]].aabb;

                for (unsigned int j=i+1;j<count;j++)
                {
                    AABB aabbj = nodes[nodeIndices[j]].aabb;
                    AABB aabb;
                    aabb.merge(aabbi, aabbj);
                    double cost = aabb.getSurfaceArea();

                    if (cost < minCost)
                    {
                        iMin = i;
                        jMin = j;
                        minCost = cost;
                    }
                }
            }

            unsigned int index1 = nodeIndices[iMin];
            unsigned int index2 = nodeIndices[jMin];

            unsigned int parent = allocateNode();
            nodes[parent].left = index1;
            nodes[parent].right = index2;
            nodes[parent].height = 1 + std::max(nodes[index1].height, nodes[index2].height);
            nodes[parent].aabb.merge(nodes[index1].aabb, nodes[index2].aabb);
            nodes[parent].parent = NULL_NODE;

            nodes[index1].parent = parent;
            nodes[index2].parent = parent;

            nodeIndices[jMin] = nodeIndices[count-1];
            nodeIndices[iMin] = parent;
            count--;
        }

        root = nodeIndices[0];

        validate();
    }

    void Tree::validateStructure(unsigned int node) const
    {
        if (node == NULL_NODE) return;

        if (node == root) assert(nodes[node].parent == NULL_NODE);

        unsigned int left = nodes[node].left;
        unsigned int right = nodes[node].right;

        if (nodes[node].isLeaf())
        {
            assert(left == NULL_NODE);
            assert(right == NULL_NODE);
            assert(nodes[node].height == 0);
            return;
        }

        assert((0 <= left) && (left < nodeCapacity));
        assert((0 <= right) && (right < nodeCapacity));

        assert(nodes[left].parent == node);
        assert(nodes[right].parent == node);

        validateStructure(left);
        validateStructure(right);
    }

    void Tree::validateMetrics(unsigned int node) const
    {
        if (node == NULL_NODE) return;

        unsigned int left = nodes[node].left;
        unsigned int right = nodes[node].right;

        if (nodes[node].isLeaf())
        {
            assert(left == NULL_NODE);
            assert(right == NULL_NODE);
            assert(nodes[node].height == 0);
            return;
        }

        assert((0 <= left) && (left < nodeCapacity));
        assert((0 <= right) && (right < nodeCapacity));

        int height1 = nodes[left].height;
        int height2 = nodes[right].height;
        int height = 1 + std::max(height1, height2);
        assert(nodes[node].height == height);

        AABB aabb;
        aabb.merge(nodes[left].aabb, nodes[right].aabb);

        for (unsigned int i=0;i<dimension;i++)
        {
            assert(aabb.lowerBound[i] == nodes[node].aabb.lowerBound[i]);
            assert(aabb.upperBound[i] == nodes[node].aabb.upperBound[i]);
        }

        validateMetrics(left);
        validateMetrics(right);
    }

    void Tree::periodicBoundaries(std::vector<double>& position)
    {
        for (unsigned int i=0;i<dimension;i++)
        {
            if (position[i] < 0)
            {
                position[i] += boxSize[i];
            }
            else
            {
                if (position[i] >= boxSize[i])
                {
                    position[i] -= boxSize[i];
                }
            }
        }
    }

    bool Tree::minimumImage(std::vector<double>& separation, std::vector<double>& shift)
    {
        bool isShifted = false;

        for (unsigned int i=0;i<dimension;i++)
        {
            if (separation[i] < negMinImage[i])
            {
                separation[i] += periodicity[i]*boxSize[i];
                shift[i] = periodicity[i]*boxSize[i];
                isShifted = true;
            }
            else
            {
                if (separation[i] >= posMinImage[i])
                {
                    separation[i] -= periodicity[i]*boxSize[i];
                    shift[i] = -periodicity[i]*boxSize[i];
                    isShifted = true;
                }
            }
        }

        return isShifted;
    }
}

#endif /* _AABB_H */
