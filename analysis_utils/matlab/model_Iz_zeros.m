function dydt = model_Iz_zeros(v, I, a, b, k, vr, vt, C)

u = b .* (v - vr);
dydt = (1 ./ C) .* (k .* (v - vr) .* (v - vt) - u + I);

