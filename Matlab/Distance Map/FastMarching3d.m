%This function takes a 3D data volume matrix as first argument
%and a threshold as second argument. Everything above aThreshold
%will serve as the source for the distance map computation.
%
%The 3D data volume must have been previously loaded, this function
%doesn't read files.
function DistanceMap = FastMarching3d(aInputVolume, aThreshold);

path(path,'Source');

%Change folder to place the compiled file in the source folder
 oldFolder = cd('Source');
 %mex msfm3d.c
 cd(oldFolder);

 if exist('aThreshold','var') == 0
     aThreshold = 0;
 end

dimensions = size(aInputVolume);
[nonZero(1,:) nonZero(2,:) nonZero(3,:)] =...
    ind2sub( dimensions, find(aInputVolume > aThreshold));

speedMap    = ones(dimensions);
DistanceMap = msfm3d(speedMap,nonZero,true,true);

;end
