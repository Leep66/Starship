This is my SD1-A4 Starship Gold.


Game Features

PLAYERSHIP CONTROLS: Use the keyboard to control the ship's navigation. The ship can rotate, propel, cloak and fire bullets.

ENEMY TYPES: Includes asteroids, beetles and wasps, each with unique behavior.

BULLETS MECHANICS: The player can fire bullets to destroy enemies. Bullets will follow the ship's current direction.

Collision System: Handles collisions between the player, enemies and bullets. Entities will push each other away when they overlap.

SPECIAL ABILITIES: The ship can fire a spread of bullets in a conical area or 360 degrees.


Controls

MOVEMENT:
Rotate the ship with the S or F keys, or use the joystick to rotate.
Use the E key or joystick to move forward.

FIRING BULLETS: Press J or Xbox Controller A to fire.

SPECIAL ABILITIES: Press K or L to use diffuse fire (X and B joysticks), and SPACE (Y joystick) to become invisible.


Known Issues:



Deep Learning:

A clear code structure and design of the relationships between classes is crucial when building the program architecture.App is the core of the application, which drives the game logic and rendering through the main loop that keeps calling Game's Update() and Render().Game is responsible for managing and updating all the Entity objects. I use Game's UpdateEntityList() to perform bulk updates. Different entity types (e.g. player, enemy, bullet, etc.) are realized by inheriting Entity class. Derived classes of each Entity class can override Update() and Render() for this purpose.

At first the code architecture was very confusing for me, I couldn't figure out what each function in MainWindows did and how it should be placed and used. I wasn't sure if I should put a certain function in a certain file. Later I realized the importance of modularizing each function, e.g., separating rendering and updating so that they can be modified and extended independently. And as the project progressed, I came to understand the functionality and interrelationships of each part, so that I could refactor the code quickly and be able to optimize it at any time.

I often encountered nagging bugs in my code that I couldn't fix after many tries. But in the end, I realized it was just a very small problem. This experience can be not only frustrating, but also potentially time-consuming. They often require a global understanding of the code and rely on step-by-step troubleshooting to find them. But step-by-step troubleshooting is the most efficient way to find problems. Using elimination and reducing the interference of irrelevant factors is how I can find the problem.

