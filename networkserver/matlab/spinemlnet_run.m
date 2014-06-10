function [] = spinemlnet_run ()
% start the interface thread
display('SpineMLNet initialising...');
snetHandle = spinemlnetStart;
display('SpineMLNet initialised.');
display (snetHandle);

% Specify a cleanup function.
cleanupObj = onCleanup(@() spinemlnetCleanup(snetHandle));

%display (snetHandle);

% First job - manually call the handshake and look at status?

% loop until the user presses 'q'
escaped = false;
%tic % start timer.
%counter = 0;
while escaped == false

    % query for current state.
    qrtn = spinemlnetQuery (snetHandle);
    % qrtn is:
    % qrtn(1,1): threadFailed
    % qrtn(1,2): updated
    %
    %display (qrtn(1,1));
    
    %pause (1);
    
    if qrtn(1,1) == 1
        % The thread failed, so set escaped to true.
        display ('The TCP/IP I/O thread seems to have failed. Finishing.');
        escaped = true;
    end
    
    % exit if the figure receives a 'q'
    %    char = get (gcf,'currentcharacter');
    %set (gcf,'currentcharacter','e');
    %if char == 'q'
    %    escaped = true;
    %end

end

% spinemlnetCleanup gets called at end.
end

function spinemlnetCleanup (snetHandle)
    disp('spinemlnetCleanup: Calling stop');
    display (snetHandle);
    spinemlnetStop(snetHandle);
    disp('Called stop');
end
