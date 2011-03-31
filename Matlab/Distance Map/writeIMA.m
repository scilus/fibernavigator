
function data = writeIMA( data, outputname, dx, dy, dz, dt )

if exist('outputname') == 0 
    outputname = 'tmp';
end

outputfullnameIMA = sprintf('%s.ima',outputname);
outputfullnameDIM = sprintf('%s.dim',outputname);

fid1 = fopen(outputfullnameIMA,'w');
fid2 = fopen(outputfullnameDIM,'w');
fwrite(fid1,data,'float');

if exist( 'dx' ) == 0
    dx = 1;
end
if exist( 'dy' ) == 0
    dy = 1;
end
if exist( 'dz' ) == 0
    dz = 1;
end
if exist( 'dt' ) == 0
    dt = 1;
end

dims = size(data);

if length(dims) == 2
    Z = 1;
else
    Z = dims(3);
end
if length(dims) == 3
    T = 1;
else
    T = dims(4);
end

fprintf(fid2,'%d %d %d %d\n', dims(1), dims(2), Z, T);
fprintf(fid2,'-type FLOAT\n');
fprintf(fid2,'-dx %f -dy %f -dz %f -dt %f\n', dx, dy, dz, dt);
fprintf(fid2,'-bo DCBA\n-om binar\n');

fclose(fid1);
fclose(fid2);

