function data = load_neural_sheet (file_path)
% Load double-formatted binary data

    isOctave = exist('OCTAVE_VERSION', 'builtin') ~= 0;

    % First, open the file:
    [ fid, fopen_msg ] = fopen (file_path, 'r', 'native');
    if fid == -1
        display (['Failed to open file ', file_path, ' with error: ', ...
                  fopen_msg]);
        count = 0; data = [];
        return;
    end

    data = [];
    while ~feof(fid)
        [ idx1, cnt ] = fread (fid, [1], 'int32');
        [ data1, cnt ] = fread (fid, [1], 'double=>double');
        data = [data data1];
    end

    % Finally, would close the file here, but it seems to fail.
end
