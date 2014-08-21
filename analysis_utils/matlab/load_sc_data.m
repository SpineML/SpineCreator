function [ data, count ] = load_sc_data (file_path, num_neurons)
% load_sc_data Code to load up data which has been output from SpineCreator.
% You have to tell this function how many time series are present in the data.

    % First, open the file:
    [ fid, fopen_msg ] = fopen (file_path, "r", "native");
    if fid == -1
        display (['Failed to open file ', file_path, ' with error: ', ...
                  fopen_msg]);
        count = 0; data = [];
        return;
    end

    % Imagine num_neurons is 4. This will return a 4 row matrix with as
    % many columns as there are timesteps in your output time
    % series. SpineCreator data is always double precision.
    [ data, count ] = fread(fid, [num_neurons, Inf], "double");

    % Finally, close the file.
    %rtn = fclose (fid);
    rtn = 0;
    if rtn == -1
        display (['Warning: failed to close file ', file_path]);
    end

end
