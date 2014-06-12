function [] = spinemlnet_run ()
% start the interface thread
display ('SpineMLNet ML: initialising...');
snetHandle = spinemlnetStart;
display ('SpineMLNet ML: initialised.');
%display (snetHandle);

% Specify a cleanup function.
cleanupObj = onCleanup(@() spinemlnetCleanup(snetHandle));

%display (snetHandle);

% First job - manually call the handshake and look at status?

% loop until the user presses 'q'
escaped = false;
%tic % start timer.
%counter = 0;
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
    
    pause (1);
    
    if qrtn(1,1) == 1
        % The thread failed, so set escaped to true.
        display ('SpineMLNet ML: The TCP/IP I/O thread seems to have failed. Finishing.');
        escaped = true;
    end
    
    % exit if the figure receives a 'q'
    %    char = get (gcf,'currentcharacter');
    %set (gcf,'currentcharacter','e');
    %if char == 'q'
    %    escaped = true;
    %end

end

display ('SpineMLNet ML: Script finished');
end

function spinemlnetCleanup (snetHandle)
    disp('SpineMLNet ML: spinemlnetCleanup: Calling spinemlnetStop');
    spinemlnetStop(snetHandle);
    disp('SpineMLNet ML: cleanup complete.');
end
