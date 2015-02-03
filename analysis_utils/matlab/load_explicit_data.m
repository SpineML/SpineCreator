function [ data, idx, count ] = load_explicit_data (file_path)
% load_explicit_data Code to load up explicitDataBinaryFiles as used
% by SpineCreator.

    isOctave = exist('OCTAVE_VERSION', 'builtin') ~= 0;

    % First, open the file:
    [ fid, fopen_msg ] = fopen (file_path, 'r', 'native');
    if fid == -1
        display (['Failed to open file ', file_path, ' with error: ', ...
                  fopen_msg]);
        count = 0; data = [];
        return;
    end

    % Explicit data binary files are signed int, double, signed
    % int, double etc etc. The signed ints are the index, the
    % double is the value (weight).
    %
    % Because we can't skip an int's worth of data between doubles,
    % have to do a simple loop to read this data.
    idx = [];
    data = [];
    count = 0;
    while ~feof(fid)
        [ idx1, cnt ] = fread (fid, [1], 'int32=>int');
        if ~feof(fid)
            idx = [idx idx1];
            [ data1, cnt ] = fread (fid, [1], 'double=>double');
            data = [data data1];
            count = count + 1;
        end
    end

    % Finally, would close the file here, but it seems to fail.
end
