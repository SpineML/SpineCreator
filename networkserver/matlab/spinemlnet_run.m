function [] = spinemlnet_run ()
% start the interface thread
display ('SpineMLNet ML: initialising...');
context = spinemlnetStart (50091);
display ('SpineMLNet ML: initialised.');

% Specify a cleanup function.
cleanupObj = onCleanup(@() spinemlnetCleanup(context));

% First job - add some data for a connection

% My sample experiment has 2 neurons in its single population and requires
% 3000 points for each neuron. Here are two sines with 3000 points an
% about 4 3/4 periods, one larger in amplitude than the other:
sine_array = 50 * sin([0:0.01:29.99]);
sine_array2 = 10 * sin([0:0.01:29.99]);

% Now make an input_data matrix up with each time series in its own row.
% Note: one row per member of the population. Time extends to the right
% with increasing column.
input_data = [sine_array;sine_array2];

% Add the sine_array data to spinemlnet for the first connection,
% which is called realtime.
[artn errormsg] = spinemlnetAddData (context, 'realtime', input_data);
if length(errormsg) > 0
    display (errormsg);
end

% Add the sine_array data to spinemlnet for a second connection,
% which is called realtime2 (not used in my example lif_test2 expt).
[artn errormsg] = spinemlnetAddData (context, 'realtime2', input_data);
if length(errormsg) > 0
    display (errormsg);
end

% loop until the spinemlnet system has finished.
escaped = false;

display ('SpineMLNet ML: Going into loop...');
while escaped == false

    %display ('SpineMLNet ML: Call spinemlnetQuery()');

    % query for current state.
    qrtn = spinemlnetQuery (context);
    % qrtn is:
    % qrtn(1,1): threadFinished (possibly failure)
    % qrtn(1,2): connectionsFinished - this means you can go ahead
    % and collect data with spinemlnetGetData().
    
    % You can also call spinemlnetAddData() within the loop, as
    % often as you like to add to the data which will be passed to
    % the SpineML experiment/simulation/execution.
    
    if qrtn(1,1) == 1
        % The thread failed, so set escaped to true.
        display ('SpineMLNet ML: The TCP/IP I/O thread seems to have failed. Finishing.');
        escaped = true;
    end

    if qrtn(1,2) == 1
        % The connections completed, so we can collect data and escape
        display ('SpineMLNet ML: Getting data...');
        myoutput = spinemlnetGetData (context, 'netout');
        %size (myoutput)
        % The larger sine wave is red
        plot (myoutput(1,:), 'r');
        hold on
        % The smaller one is seen in the blue trace
        plot (myoutput(2,:), 'b');
        hold off
        escaped = true;
    end

    pause (1);
end

display ('SpineMLNet ML: Script finished');
end

function spinemlnetCleanup (context)
    disp('SpineMLNet ML: spinemlnetCleanup: Calling spinemlnetStop');
    spinemlnetStop(context);
    disp('SpineMLNet ML: cleanup complete.');
end
