
function data = loadIMA( file, Nx, Ny, Nz)

fid = fopen(file);
data = fread(fid, inf, 'uint16'); 
data = reshape(data,[Nx,Ny,Nz]);
fclose(fid)



