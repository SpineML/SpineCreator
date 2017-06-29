%%
%% Code to load up binary connection lists as written out by
%% SpineML_PreFlight and as consumed by SpineML_2_BRAHMS.
%%
%% You have to tell it where the connection list file is and
%% whether or not that file contains delays, as well as src/dst
%% index pairs.
%%
%% Usage:
%%
%% [ srcdest, dly, count ] = load_connection_list (file_path, has_delay)
%%
%% srcdest: a matrix of ints with two cols, the first are the src
%% indices, the second are the dest indices.
%%
%% dly: A vector (column) of the delays.
%%
%% count: How many src/dest pairs were read.
%%
function [ srcdest, dly, count ] = load_connection_list (file_path, has_delay)

    isOctave = exist('OCTAVE_VERSION', 'builtin') ~= 0;

    % First, open the file:
    [ fid, fopen_msg ] = fopen (file_path, 'r', 'native');
    if fid == -1
        display (['Failed to open file ', file_path, ' with error: ', ...
                  fopen_msg]);
        count = 0; data = [];
        return;
    end

    % Connection lists are EITHER int int float, int int float, etc
    % OR just int int, int int, int int etc. The float is an
    % optional delay. User has to say whether it has delay or not.
    sidx = [];
    didx = [];
    dly = [];
    count = 0;
    while ~feof(fid)
        count = count + 1;
        [ sidx1, cnt ] = fread (fid, [1], 'int32=>int');
        sidx = [sidx; sidx1];
        if ~feof(fid)
            [ didx1, cnt ] = fread (fid, [1], 'int32=>int');
            didx = [didx; didx1];
        end
        if ~feof(fid) && has_delay
            [ dly1, cnt ] = fread (fid, [1], 'float=>double');
            dly = [dly; dly1];
        end
    end

    srcdest = [sidx, didx];

    % Finally, would close the file here, but it seems to fail.
end
