function [stem_x, stem_y, arrow1_x, arrow1_y, arrow2_x, arrow2_y, lens] = quiver(x, y, ux, uy, scale, do_pow)

% need very small scale (0.05 or less when using power)

hold off
alpha = 0.4;
theta = 30; % degrees
power = 0.42;

r_theta = (pi ./ 180) .* theta;

% plot stems
[r c] = size(x);
no_els  = r .* c;

Vx = reshape(x, 1, no_els);
dx = x(1,1) - x(1,2);
dx = abs(dx);

Vy = reshape(y, 1, no_els);
dy = y(1,1) - y(2,1);
dy = abs(dy);

% Now convert to a square axis set for calulation (convert back again
% later)
range_x = max(Vx) - min(Vx);
range_y = max(Vy) - min(Vy);
beta = range_x ./ range_y;


Vy = beta .* Vy;

Vux = reshape(ux, 1, no_els);
Vuy = reshape(uy, 1, no_els);


max_ux = max(abs(Vux));
max_uy = max(abs(Vuy));
dd = min([dx dy]);

max_u = max([max_ux max_uy]);

Vux = scale .* (Vux ./ max_u) .* dd;

breaker = repmat(NaN, size(Vx));



Vuy = beta .* scale .* (Vuy ./ max_u) .* dd;

lens = max_u .* sqrt(Vux .* Vux + Vuy .* Vuy);
if do_pow
    Vux = Vux .* (lens .^ (power - 1));
    Vuy = Vuy .* (lens .^ (power - 1));
end

stem_x = [Vx; Vx + Vux; breaker]; % 3 by no_els
stem_x = reshape(stem_x, 1, 3 .* no_els);

stem_y = [Vy; Vy + Vuy; breaker]; % 3 by no_els
stem_y = (1 ./ beta) .* reshape(stem_y, 1, 3 .* no_els);

plot(stem_x, stem_y)
hold on

%%%%%%%% plot arrows

c = cos(r_theta);
s = sin(r_theta);

h1x = alpha .* (c .* Vux - s .* Vuy);
h1y = alpha .* (s .* Vux + c .* Vuy);
h2x = alpha .* (c .* Vux + s .* Vuy);
h2y = alpha .* (-s .* Vux + c .* Vuy);

end_a1x = Vx + Vux - h2x;
end_a1y = Vy + Vuy - h2y;
end_a2x = Vx + Vux - h1x;
end_a2y = Vy + Vuy - h1y;

arrow1_x = [Vx + Vux; end_a1x; breaker];
arrow1_x = reshape(arrow1_x, 1, 3 .* no_els);

arrow1_y = [Vy + Vuy; end_a1y; breaker];
arrow1_y = (1 ./ beta ) .* reshape(arrow1_y, 1, 3 .* no_els);

arrow2_x = [Vx + Vux; end_a2x; breaker];
arrow2_x = reshape(arrow2_x, 1, 3 .* no_els);

arrow2_y = [Vy + Vuy; end_a2y; breaker];
arrow2_y = (1 ./ beta) .* reshape(arrow2_y, 1, 3 .* no_els);

plot(arrow1_x, arrow1_y)
plot(arrow2_x, arrow2_y)

hold off
axis square

