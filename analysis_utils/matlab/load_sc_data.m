function [ data, count, t ] = load_sc_data (varargin)
% load_sc_data Code to load up data which has been output from SpineCreator.
% Returns the time axis in milliseconds in the variable t.

% For Octave, and XML support (using load_sc_data with one argument),
% you need these jar files from e.g.
% https://www.apache.org/dist/xerces/j/Xerces-J-bin.2.11.0.tar.gz
%
% you need use javaaddpath to include these files:
% javaaddpath('/home/you/Downloads/xerces-2_11_0/xercesImpl.jar');
% javaaddpath('/home/you/Downloads/xerces-2_11_0/xml-apis.jar');
%
% Further down in your copy of this code, change the javaaddpath calls
% and then change the gotXerces variable to 1.
    
% varargin can be filled with arguments file_path then optionally
% num_neurons (which avoids use of xml reading code).

    isOctave = exist('OCTAVE_VERSION', 'builtin') ~= 0;

    if nargin == 1
        % No n_neurons, so only extract file_path
        file_path = varargin{1};
        num_neurons = [];
    elseif nargin == 2
        file_path = varargin{1};
        num_neurons = varargin{2};
    else
        display ('Error: Wrong number of arguments');
        return;
    end

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

    % Timestep size. Initialise to 0.
    dt = 0;

    % Init some parameters which may be read from the XML file
    logFileType = 'binary';
    logEndTime = 0;
    logType = 'double';
    logPort = '';
    % Obtained from logCol elements:
    logColHeadings = [];
    logColTypes = [];
    
    if (isempty(num_neurons))
        % Find the number of neurons in the binary log file from
        % the xml file.
        if isOctave
            % User! You're using octave and you have asked for XML reading of the
            % SpineML log metadata, so need to javaaddpath.
            gotXerces = 1; % User! Change this to 1 when you're done!
            if gotXerces == 0
                display (['Calling this function with 1 arguments means ' ...
                          'it needs to read XML. For XML support  you need ' ...
                          'to get Xerces and modify the javaaddpath() ' ...
                          'lines in the code. Please see the ' ...
                          'load_sc_data.m code.']);
                return;
            end
            % User! Modify these javaaddpath lines to match the Xerces you downloaded!
            javaaddpath('/home/seb/ownCloud/octave/xerces-2_11_0/xercesImpl.jar');
            javaaddpath('/home/seb/ownCloud/octave/xerces-2_11_0/xml-apis.jar');


            % these 3 lines are equivalent to infoDoc = xmlread (xml_file)
            parser = javaObject ('org.apache.xerces.parsers.DOMParser');
            parser.parse (xml_file);
            infoDoc = parser.getDocument;
        else
            infoDoc = xmlread (xml_file);
        end

        % Assume Analog Log here. Probably wrong for event log.
        logFileType = char(infoDoc.getElementsByTagName ...
                           ('LogFileType').item(0).getFirstChild ...
                           .getData);
        display (logFileType)

        if strcmp(logFileType, 'csv') || strcmp(logFileType, 'binary')
            % ok
        else
            display (['File described by ', xml_path, ' is not marked ' ...
                      'as being in binary or csv format.']);
            return;
        end

        % Log end is in steps of size dt. Unused at present even though
        % this is in the UI? Also may need logStartTime in future to
        % generate t.
        logEndTime = str2num(char(infoDoc.getElementsByTagName ...
                                  ('LogEndTime').item(0).getFirstChild.getData));
        num_neurons = str2num(char(infoDoc.getElementsByTagName ...
                                   ('LogAll').item(0).getAttribute('size')));

        logType = char(infoDoc.getElementsByTagName ...
                       ('LogAll').item(0).getAttribute('type'));
        
        %logHeading = char(infoDoc.getElementsByTagName ...
        %                  ('LogCol').item(0).getAttribute('heading'))
        
        % Timestep is specified in milliseconds
        dt = str2num(char(infoDoc.getElementsByTagName ...
                          ('TimeStep').item(0).getAttribute('dt')));    

    end % else num_neurons already set.

    if strcmp(logFileType, 'binary') % The log file is binary format.

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
        
    else % The log file is in csv format
        
        display ('Warning, csv file type not yet supported.');
    
    end
end
