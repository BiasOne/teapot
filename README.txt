Vulkan Mario World

All of the shaders and generated files are included in this ZIP. If you wish to regenerate or rebuild, run the following:
    make clean
    make shader
    make
    make run

Functionality
    A Vulkan-based 3D renderer featuring an interactive camera system and a custom Mario World
    scene.

Camera Controls
    R           - Rotate mode   (drag mouse to rotate around scene center)
    P           - Pan mode      (drag mouse to pan across the scene)
    Z           - Zoom mode     (drag mouse to zoom in and out)
    T           - Twist mode    (drag mouse to twist/roll the camera)
    F           - Fit All       (resets camera to frame the entire scene around Mario)

Object Controls
    WASD        - Move Mario through the scene
    C           - Toggle between perspective and orthographic projection
    ESC         - Exit

Creative Components
    Mario World Scene: A Mario-themed environment that includes a grassy ground plane, clouds, hills, trees, green pipes with piranha plants,
    floating question mark and brick blocks, coins, Mushrooms, Goombas, and Koopas.

    Mario Game Object: A custom blocky Mario character.

    Dynamic Fit All: The Fit All function (F key) tracks Mario's current position as he moves
    through the scene, always reframing the camera to keep Mario centered.