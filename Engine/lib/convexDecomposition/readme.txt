The ConvexDecomposition library was written by John W. Ratcliff mailto:jratcliffscarab@gmail.com

What is Convex Decomposition?

Convex Decomposition is when you take an arbitrarily complex triangle mesh and sub-divide it into
a collection of discrete compound pieces (each represented as a convex hull) to approximate
the original shape of the objet.

This is required since few physics engines can treat aribtrary triangle mesh objects as dynamic
objects.  Even those engines which can handle this use case incurr a huge performance and memory
penalty to do so.

By breaking a complex triangle mesh up into a discrete number of convex components you can greatly
improve performance for dynamic simulations.

--------------------------------------------------------------------------------

This code is released under the MIT license.

The code is functional but could use the following improvements:

(1) The convex hull generator, originally written by Stan Melax, could use some major code cleanup.

(2) The code to remove T-junctions appears to have a bug in it.  This code was working fine before,
	but I haven't had time to debug why it stopped working.

(3) Island generation once the mesh has been split is currently disabled due to the fact that the
	Remove Tjunctions functionality has a bug in it.

(4) The code to perform a raycast against a triangle mesh does not currently use any acceleration
	data structures.

(5) When a split is performed, the surface that got split is not 'capped'.  This causes a problem
	if you use a high recursion depth on your convex decomposition.  It will cause the object to
	be modelled as if it had a hollow interior.  A lot of work was done to solve this problem, but
	it hasn't been integrated into this code drop yet.


