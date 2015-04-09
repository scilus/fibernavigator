Fibernavigator
==============

The Fibernavigator is a tool designed for a fast and versatile visualization of fibers datasets.

**Welcome new users**

You can find a pre-build executable (windows) and all the latest tools here: https://github.com/chamberm/fibernavigator/releases.

Please note that the Fibernavigator now has a Real-time fMRI connectivity mode and
new anatomy coloring modes. The branch containing this code is either **Master** or **Tracto-driven**,
and is based on wxWidgets 3.0. The branch **opacity-rendering** has implementation of Tax et al. 2014 ISMRM abstract 
for the interactive rendering of orientation-dependent tractography. Be sure to visit https://github.com/chamberm/fibernavigator/releases.

Tutorials
---------

New **video tutorials** of the Fibernavigator are available.

- `Scene navigation`_: how to interact with the 3D view, move planes, etc.
- `Mesh display and interaction`_: how to create and modify the display of meshes.
- `Drawing tool`_: how to draw on empty or already existing anatomies.
- `Fiber navigation`_: how to interact with fibers and select a subset of them.

A list of all tutorials can be seen on the `video tutorials`_ page.

Highlights:
-----------

- Fast and efficient visualization of the results of deterministic and probabilistic tractograms.
- Interactive selection of fibers.
- Real-time fiber tractography. (Chamberland et al. 2014, Frontiers in Neuroinformatics)
- Tractography-driven resting-state fMRI (Chamberland et al. 2015, Proceeding of OHBM)
- Orientation-dependent opacity rendering (Tax et al. 2014, ISMRM)
- Display of MRI, functional MRI and EEG/MEG source localization results.

How to get the application
--------------------------

- `Build instructions for Ubuntu`_ and other Linux distros are provided
- Windows and OSX builds are available on the Releases_ page.

Development and contact with the team
-------------------------------------

- Issues can be submitted in the `issue tracker`_.
- Various development and use recommandations and how-tos are located on the wiki_.


**A note for previous users**

This is the new source for the code and documentation of the `original Fibernavigator`_.

All issues have been moved to the new `issue tracker`_. New issues will also be tracked here.

.. _original Fibernavigator: http://code.google.com/p/fibernavigator/
.. _issue tracker: https://github.com/scilus/fibernavigator/issues
.. _video tutorials: https://github.com/scilus/fibernavigator/wiki/Video-tutorials
.. _Build instructions for Ubuntu: https://github.com/scilus/fibernavigator/wiki/Ubuntu-build-instructions
.. _Releases: https://github.com/chamberm/fibernavigator/releases
.. _wiki: https://github.com/scilus/fibernavigator/wiki/_pages
.. _Scene navigation: http://www.youtube.com/watch?v=OXuHX8GGaBQ
.. _Mesh display and interaction: http://www.youtube.com/watch?v=VONdX7iTNSI
.. _Drawing tool: http://www.youtube.com/watch?v=4vYkQLrdYaY
.. _Fiber navigation: http://www.youtube.com/watch?v=8c4Smi9gZOA

Acknowledgements
-------------------------------------
If you use the Fibernavigator, please make sure that you quote the following reference in any publications:

*Chamberland M., Whittinstall K., Fortin D., Mathieu D., and Descoteaux M., "Real-time multi-peak tractography for instantaneous connectivity display." Frontiers in Neuroinformatics, 8 (59): 1-15, 2014.*
