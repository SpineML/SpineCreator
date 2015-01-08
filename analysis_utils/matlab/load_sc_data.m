function [ t, data, count ] = load_sc_data (file_path)
% load_sc_data Code to load up data which has been output from SpineCreator.
% Returns the time axis in milliseconds in the variable t.

    % If file path ends with .bin, or .xml then find the root name.
    bin_end = file_path (end-3:end);
    xml_end = file_path (end-6:end);
    if bin_end == '.bin'
        base_path = file_path (1:end-4);
    elseif xml_end == 'rep.xml'
        base_path = file_path (1:end-7);
    else
        % Error
        display (['Error: Bad SpineCreator log file name: ', ...
                  file_path]);
        return;
    end

    xml_file = [base_path 'rep.xml'];
    bin_file = [base_path '.bin'];

    % Find the number of neurons in the binary log file from
    % the xml file.
    infoDoc = xmlread (xml_file);
    % Assume Analog Log here. Probably wrong for event log.
    logFileType = char(infoDoc.getElementsByTagName ...
                       ('LogFileType').item(0).getFirstChild ...
                       .getData);
    if logFileType ~= 'binary'
        display (['File described by ', xml_path, ' is not marked ' ...
                  'as being in binary format.']);
        return;
    end

    % Log end is in steps of size dt. Unused at present even though
    % this is in the UI? Also may need logStartTime in future to
    % generate t.
    logEndTime = str2num(char(infoDoc.getElementsByTagName ...
                              ('LogEndTime').item(0).getFirstChild.getData));
    num_neurons = str2num(char(infoDoc.getElementsByTagName ...
                               ('LogAll').item(0).getAttribute('size')));
    % Timestep is specified in milliseconds
    dt = str2num(char(infoDoc.getElementsByTagName ...
                      ('TimeStep').item(0).getAttribute('dt')));

    % First, open the file:
    [ fid, fopen_msg ] = fopen (bin_file, 'r', 'native');
    if fid == -1
        display (['Failed to open file ', file_path, ' with error: ', ...
                  fopen_msg]);
        count = 0; data = [];
        return;
    end

    % Imagine num_neurons is 4. This will return a 4 row matrix with as
    % many columns as there are timesteps in your output time
    % series. SpineCreator data is always double precision.
    [ data, count ] = fread (fid, [num_neurons, Inf], 'double=>double');

    % Construct time series in milliseconds.
    t = [0 : dt : (dt*count)-dt];

    % Finally, close the file.
    %rtn = fclose (fid);
    rtn = 0;
    if rtn == -1
        display (['Warning: failed to close file ', file_path]);
    end

end
