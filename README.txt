20196014 고지흔
OS : WINDOW 10 
made by Visual studio 2019

Run VirtualLego.sln, set the path, and debug.
If it doesn't run properly, replace my virtualLego.cpp and d3dUtility.h, d3dUtility.cpp with your running folder.

You can move the blue ball at the bottom with the left mouse and the white ball at the top with the left mouse.
Space can be used to fire magenta balls.
*Do not press the space bar with the left mouse pressed. The cause is unknown, but the position of the blue ball moves quickly.

The white ball serves to show the path through which the magenta ball is first fired. It disappears after that.

When a magenta ball collides with a yellow ball, the yellow ball disappears.

If you get rid of all the yellow balls, you clear the game, and if the magenta ball touches the floor, the game fails. 
In this case, the magenta ball disappears and the program must be restarted to play the game again.

Summary of my Code Modification

-I set the shooting ball as the agenta ball and fixed the z-coordinate.
-The direction of firing the magenta ball was set as a white ball, and the white ball also fixed the z-coordinate. 
-After the first launch, the white ball disappears and nothing happens when you press space.
-I changed the bottom wall to green and made the mangenta ball disappear when it crashed into the bottom wall.
-The speed of the ball has changed to a constant since start.
-I made hasIntersected considering the distance between the ball and the wall. Hitby was made by considering the direction of the ball after the collision.
-Adjusted the size of the billiard table and the camera angle.
-When the magenta ball collides with the yellow ball, the yellow ball disappears and the magenta ball disappears when all the yellow balls disappear.