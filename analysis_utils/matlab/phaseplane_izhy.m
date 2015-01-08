% A function to take u and v from an Izhykevich neuron simulation
% carried out in SpineCreator and show phase plane and quiver plots
% and analysis. Based on Kevin Gurney's demo matlab code for the MS
% course - specifically dynamicsIz.m with the simulation removed
% from the code.
%
% Adapted by Seb James

function phaseplane_izhy (spineml_results_path, experimentXml, ...
                          populationName, paramNames, stateVarNames, ...
                          quiverParams)

    % Hard-coded parameters.
    % Initial value for locating an equilibrium
    initV = -45;
    % For nullclines:
    lowerV = -70;
    upperV = 50;

    % Initialise model parameters and load them from the model.xml
    a = 0; b = 0; c = 0; d = 0;
    Cap = 0; vr = 0; vt = 0; k = 0; Vpeak = 0;
    izhy_cmpt = '';
    modelDoc = xmlread([spineml_results_path, '/model/model.xml']);
    neurons = modelDoc.getElementsByTagName ('LL:Neuron');
    % find the neuron which is called populationName.
    for n_iter = 0:neurons.getLength-1
        neuron = neurons.item (n_iter);
        nm = neuron.getAttribute ('name');
        if nm == populationName
            % This is our population. Can get component xml name here.
            izhy_cmpt = neuron.getAttribute ('url');
            % Get all paramNames.
            % get value of paramNames('a') % etc
            props = neuron.getElementsByTagName ('Property');
            for p_iter = 0:props.getLength-1
                property = props.item (p_iter);
                pname = property.getAttribute ('name');
                if pname == paramNames('a')
                    % Assume this param property is a fixed value
                    a = str2num(char(property.getElementsByTagName ...
                                     ('FixedValue').item (0).getAttribute ('value')));
                elseif pname == paramNames('b')
                    b = str2num(char(property.getElementsByTagName ...
                                     ('FixedValue').item (0).getAttribute ('value')));
                elseif pname == paramNames('c')
                    c = str2num(char(property.getElementsByTagName ...
                                     ('FixedValue').item (0).getAttribute ('value')));
                elseif pname == paramNames('d')
                    d = str2num(char(property.getElementsByTagName ...
                                     ('FixedValue').item (0).getAttribute ('value')));
                elseif pname == paramNames('Cap')
                    Cap = str2num(char(property.getElementsByTagName ...
                                       ('FixedValue').item (0).getAttribute ('value')));
                elseif pname == paramNames('vr')
                    vr = str2num(char(property.getElementsByTagName ...
                                      ('FixedValue').item (0).getAttribute ('value')));
                elseif pname == paramNames('vt')
                    vt = str2num(char(property.getElementsByTagName ...
                                      ('FixedValue').item (0).getAttribute ('value')));
                elseif pname == paramNames('k')
                    k = str2num(char(property.getElementsByTagName ...
                                     ('FixedValue').item (0).getAttribute ('value')));
                elseif pname == paramNames('Vpeak')
                    Vpeak = str2num(char(property.getElementsByTagName ...
                                         ('FixedValue').item (0).getAttribute ('value')));
                    %else
                    %fprintf ('unknown component property %s\n', char(pname));
                end
            end
        end
    end

    % Now load time series.
    [t, v, v_count] = load_sc_data ([spineml_results_path ...
                        '/log/' populationName '_' stateVarNames('v') ...
                        '_log.bin']);

    [~, u, u_count] = load_sc_data ([spineml_results_path ...
                        '/log/' populationName '_' stateVarNames('u') ...
                        '_log.bin']);
    
    % The existing script uses a container y to store the data:
    y = [v', u'];
    t = t';
    
    % Duration of simulation in milliseconds
    duration = t(end)
    
    % A time series for I (much bigger than required)
    I = zeros (1, v_count);
    
    % Read the Currents from the experiment file.
    exptDoc = xmlread([spineml_results_path '/model/' experimentXml]);
    % I may be time varying:
    %
    % <TimeVaryingInput target="Population" port="I" name="I">
    %  <TimePointValue time="0" value="0"/>
    %  <TimePointValue time="100" value="100"/>
    %  <TimePointValue time="200" value="10"/>
    % </TimeVaryingInput>
    %
    % Or constant:
    %
    % <ConstantInput target="Population" port="I" value="100" name="I"/>
    
    % First check for constant input I
    constI = NaN;
    constInputs = exptDoc.getElementsByTagName ('ConstantInput');
    for ci_iter = 0:constInputs.getLength-1
        constInput = constInputs.item (n_iter);
        port = char(constInput.getAttribute ('port'));
        targ = char(constInput.getAttribute ('target'));
        if port == stateVarNames('I')
            if targ == populationName
            % Match
            constI = str2num(char(constInput.getAttribute ...
                                  ('value')))
            I = I + constI;
            end
        end
    end
    
    % If there was no constant input, then find a time varying one.
    multiI = [];
    if isnan(constI)
        % Check time series.
        tvInputs = exptDoc.getElementsByTagName ('TimeVaryingInput');
        timePointValues = [];
        for tvi_iter = 0:tvInputs.getLength-1
            tvInput = tvInputs.item (tvi_iter);
            port = char(tvInput.getAttribute ('port'));
            targ = char(tvInput.getAttribute ('target'));
            if port == stateVarNames('I')
                if targ == populationName
                    % Match. Get points now.                    
                    tpValues = tvInput.getElementsByTagName ...
                        ('TimePointValue');
                    timePointValues = zeros (tpValues.getLength, 2);
                    for tpv_iter = 0:tpValues.getLength-1
                        tpValue = tpValues.item(tpv_iter);
                        timePointValues(tpv_iter+1,1) = ...
                            str2num(char(tpValues.item(tpv_iter).getAttribute('time')));
                        timePointValues(tpv_iter+1,2) = ...
                            str2num(char(tpValues.item(tpv_iter).getAttribute('value'))); 
                    end
                end
            end
        end

        % Add final value to timePointValues to use interp1 to
        % generate I series data.
        timePointValues = [timePointValues; [t(end), timePointValues(end,2)]];
        
        I = interp1 (timePointValues(:,1), timePointValues(:,2), t, ...
                     'previous');

        % Set the "constant I" used to plot nullclines to the max
        % value of I for now. Would ideally plot multiple nullclines.
        constI = max(timePointValues(:,2));
        
        multiI = timePointValues(:,2);
    
    end

    % First plot y(:,1), the membrane voltage and I against sample
    figure(1)
    subplot(2,1,1);
    plot(t, y(:,1), 'r')
    subplot(2,1,2);
    plot(t, I)

    % find max and min for bifurcation diagram
    maxV = max(y(:,1));
    minV = min(y(:,1));
    fprintf(1, 'maximum Vm %.6fmV \n', maxV); 
    fprintf(1, 'minimum Vm %.6fmV \n\n', minV);

    %  nullclines
    figure(2)
    Vn = linspace (lowerV, upperV, 1000);

    if (isempty(multiI))
        wv = k .* (Vn - vr) .* (Vn - vt) + constI;
        plot(Vn, wv, 'b')
        hold on
    else
        for i = multiI'
            i
            wv = k .* (Vn - vr) .* (Vn - vt) + i;
            plot(Vn, wv, 'b')
            hold on
        end
    end
    ww = b .* (Vn - vr);

    plot(Vn, ww, 'r')

    % phase trajectory
    plot(y(:,1), y(:,2),  'g')
    axis([lowerV upperV -100 200]);

    hold off

    %  find zeros
    warning off MATLAB:fzero:UndeterminedSyntax

    opts = optimset('fzero');
    optimset(opts, 'TolX', 1e-25);


    [vz, fval, exitflag] = fzero(@model_Iz_zeros, initV, opts, constI, a, b, k, vr, vt, Cap);
    if exitflag > 0
        nz = b .* (vz - vr);
        fprintf(1, 'equilibrium potential %.6f \n', vz); 
        fprintf(1, 'equilibrium n gate %.6f \n', nz); 
    else
        fprintf(1, 'There were no equilibria\n\n');
    end


    % vector field around equilibrium

    % quiver plot params
    qpscale = 20; % An overall scale factor for the quiver plot
    qpinc = 0.5 % increment
    deltaV = qpinc;
    No_dvs_neg = qpscale; % Number of grid points hyperpolarisd wrt eqm point
    No_dvs_pos = No_dvs_neg;
    deltan = qpinc;
    No_dn_neg = qpscale;
    No_dn_pos = No_dn_neg;
    quiver_plot_scale = qpscale ./ 5;
    % end quiver plot params

    if exitflag < 0
        vz = -50;
        nz = 0;
    end

    v0 = vz + deltaV ./ 2;
    lv = v0 - No_dvs_neg .* deltaV;
    uv = v0 + No_dvs_pos .* deltaV;
    Vgv = lv:deltaV:uv; 
    n0 = nz + deltan ./ 2;
    ln = n0 - No_dn_neg .* deltan;
    un = n0 + No_dn_pos .* deltan;
    ngv = ln:deltan:un;

    % OR use this for complete trajectory space
    % Vgv = linspace(-2, 2, 35);
    % upper_n = 1;
    % lower_n = -1;
    % ngv = linspace(lower_n, upper_n, 35);

    [Vg, ng] = meshgrid(Vgv, ngv);

    dv = (1 ./ Cap) .* (k .* (Vg - vr) .* (Vg - vt) - ng + constI);
    dn = a .* (b .* (Vg - vr) - ng);

    figure(5)
    do_power = 1;
    [stem_x, stem_y, arrow1_x, arrow1_y, arrow2_x, arrow2_y, vlens] = KG_quiver(Vg, ng, dv, dn, quiver_plot_scale, do_power);
    hold on
    v = axis;
    plot(vz, nz, 'or')
    plot(y(:,1), y(:,2), 'r')
    axis(v);
    hold off
    max_len = max(vlens);
    fprintf('maximum vector length in vector field is %.5g\n', max_len);

    % find eigenvalues for give eqm

    if exitflag > 0
        aa = (k ./ Cap) .* (2 .* vz - (vr + vt));
        bb = 1 ./ Cap;
        cc = a .* b;
        dd = -a;
        Jac = [aa bb; cc dd];
        % [eigvec, eigval] = eig(Jac, 'nobalance');
        [eigvec, eigval] = eig(Jac);
        
        e1 = eigval(1,1);
        e2 = eigval(2,2);
        if isreal(e1) % real
            fprintf(1, 'real eigenvalues %.6f \t and %.6f\n', e1, e2); 
            if e1 .* e2 > 0 % same sign
                if e1 > 0 %  positive
                    fprintf(1, 'unstable node\n');
                else %  negative
                    fprintf(1, 'stable node\n');
                    figure(2)
                    hold on
                    [yy, j] = min(abs([e1 e2]));
                    %plot([vz vz + eigvec(1,j)], [nz nz + eigvec(2,j)], 'k')
                    hold off
                end
            else % opposite sign
                fprintf(1, 'saddle\n');
            end
        else % complex
            fprintf(1, 'complex eigenvalues %.6f %.3i \t and %.6f %.6i\n', real(e1), imag(e1), real(e2), imag(e2));
            if real(e1) > 0
                fprintf(1, 'unstable focus\n');
            else
                fprintf(1, 'stable focus\n');
            end
        end
    else
        disp('no equilibria to find eigenvalues for')
    end


    % find firing rate

    spike_thresh = 0;

    V = y(:,1);
    V_binary = V > spike_thresh;
    mask = [-1 1];
    cmask = abs(conv(+V_binary, mask));
    No_spikes = sum(cmask) ./ 2;
    fprintf(1, 'Number of spikes %.1f\n', No_spikes);
    rate = 1000 .* No_spikes ./ duration;
    fprintf(1, 'Spike rate %.5f spikes/sec\n', rate);

end
