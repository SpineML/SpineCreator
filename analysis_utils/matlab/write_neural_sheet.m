%% Write neuralsheet out to file_path as doubles in a format
%% suitable for input to Brahms.
%%
%% Usage:
%%
%%  write_neural_sheet (neuralsheet, file_path)
function write_neural_sheet (neuralsheet, file_path)

    nfs = size(neuralsheet);
    d = zeros (nfs(1), nfs(2));
    numneurons = nfs(1).*nfs(2);

    d(:,:) = 1;
    d = d .* neuralsheet;

    d = reshape (d, numneurons, 1, []);

    [fid, errmsg] = fopen (file_path, 'w');
    if fid == -1
        display (errmsg);
        return
    end

    for i = 0:numneurons-1
        % index:
        fwrite (fid, i, 'int32');
        % weight value:
        fwrite (fid, d(i+1), 'double');
    end

    fclose (fid);

end
