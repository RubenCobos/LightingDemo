Lighting Demo
=============

Project
-------

This project draws a simple scene of 3D geometric objects with lighting using DirectX 12.

Platform
--------

The project was built on Microsoft Visual Studio Community 2022 using Windows SDK 10.0.26100.0.

Libraries: dxgi.lib, dxguid.lib, d3d12.lib, d3dcompiler.lib.

Hardware Requirements
---------------------

Windows 10 (version 1507 or later)

Direct 3D feature level 12

Resolution: 1024x768

Refresh Rate: 60Hz

Usage
-----

To start, simply run the executable file. The camera is able to move forward, backward, left and right
using the w, s, a, and d keys respectively. To look, click and hold the left mouse button while aiming.
You can also move up with the space bar and down with the c key.

The compiled shader bytcode files (vs.cso and ps.cso) must be in the same location as the executable file.

Screenshots
-----------

<img width="1069" height="824" alt="lighting" src="https://github.com/user-attachments/assets/e57ee5f9-ddcd-4bb9-b301-26b14b43ef6d" />

<img width="1066" height="834" alt="wireframe" src="https://github.com/user-attachments/assets/afa44c9e-4ea1-4d5d-82c2-9a30ca919fc7" />

Acknowledgments
---------------

The project implementation is based on techniques presented in the book "Introduction to 3D Game Programming
with DirectX 12" by Frank D. Luna.
