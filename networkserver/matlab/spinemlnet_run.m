function [] = spinemlnet_run ()
% start the interface thread
display ('SpineMLNet ML: initialising...');
snetHandle = spinemlnetStart;
display ('SpineMLNet ML: initialised.');

% Specify a cleanup function.
cleanupObj = onCleanup(@() spinemlnetCleanup(snetHandle));

% First job - manually call the handshake and look at status?

% loop until the user presses 'q'
escaped = false;

display ('SpineMLNet ML: Going into loop...');
while escaped == false

    display ('SpineMLNet ML: Call spinemlnetQuery()');

    % query for current state.
    qrtn = spinemlnetQuery (snetHandle);
    % qrtn is:
    % qrtn(1,1): threadFailed
    % qrtn(1,2): updated
    %
    % display (qrtn(1,1));
    
    % Here's how we add data to a connection. You have to test the
    % return to make sure the data got added to an established connection.
    [artn errormsg] = spinemlnetAddData (snetHandle, 'realtime', [1.0 2.0 4.0 8.0])
    
    pause (1);
    
    if qrtn(1,1) == 1
        % The thread failed, so set escaped to true.
        display ('SpineMLNet ML: The TCP/IP I/O thread seems to have failed. Finishing.');
        escaped = true;
    end

end

display ('SpineMLNet ML: Script finished');
end

function spinemlnetCleanup (snetHandle)
    disp('SpineMLNet ML: spinemlnetCleanup: Calling spinemlnetStop');
    spinemlnetStop(snetHandle);
    disp('SpineMLNet ML: cleanup complete.');
end
