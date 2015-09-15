% Write neuralsheet out to file_path as doubles in a format
% suitable for input to Brahms.
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

    for i = 1:numneurons
        %fwrite (fid, i, 'int32');
        fwrite (fid, d(i), 'double');    
    end

    fclose (fid);

end
