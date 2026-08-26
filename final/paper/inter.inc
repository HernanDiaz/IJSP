
\begin{table}[t]
\centering
\caption{Mean RPD (\%) restricted to the instances where the control
\emph{and} the compared arm both satisfy the plateau rule. ``$n$ both'' is
the size of that intersection, which differs by arm because convergence
rates do. Negative $\Delta$ favours the seeded arm. This is the comparison
that conditioning on the control alone cannot support, since a
control-plateau instance need not be one in which the seeded arm has
plateaued too.}
\label{tab:inter}
\small
\begin{tabular}{llcccc}
\toprule
Solver & Arm & $n$ both & A0 & Arm & $\Delta$ \\
\midrule
\multirow{6}{*}{GA}
 & V2H & 9  & 4.47 & 3.85 & $-0.62$ \\
 & V2  & 11 & 4.44 & 4.04 & $-0.40$ \\
 & MOR & 8  & 3.88 & 5.98 & $+2.10$ \\
 & GT  & 5  & 4.23 & 5.51 & $+1.28$ \\
 & GP  & 9  & 4.27 & 3.64 & $-0.63$ \\
 & MIX & 10 & 4.14 & \textbf{3.45} & $\mathbf{-0.69}$ \\
\midrule
\multirow{6}{*}{ABCE3}
 & V2H & 46 & 11.61 & 10.46 & $-1.15$ \\
 & V2  & 46 & 11.61 & \textbf{10.34} & $\mathbf{-1.27}$ \\
 & MOR & 43 & 11.69 & 11.95 & $+0.26$ \\
 & GT  & 44 & 11.69 & 11.36 & $-0.34$ \\
 & GP  & 45 & 11.64 & 10.73 & $-0.91$ \\
 & MIX & 46 & 11.61 & 10.35 & $-1.26$ \\
\midrule
\multirow{6}{*}{fEABCLS}
 & V2H & 27 & 3.54 & 3.53 & $-0.00$ \\
 & V2  & 31 & 3.26 & 3.22 & $\mathbf{-0.05}$ \\
 & MOR & 19 & 3.86 & 5.27 & $+1.41$ \\
 & GT  & 27 & 3.49 & 4.11 & $+0.62$ \\
 & GP  & 23 & 2.62 & 2.81 & $+0.20$ \\
 & MIX & 34 & 3.46 & 3.48 & $+0.02$ \\
\midrule
\multirow{6}{*}{TSN2}
 & V2H & 35 & 2.37 & 2.41 & $+0.04$ \\
 & V2  & 30 & 2.49 & 2.57 & $+0.08$ \\
 & MOR & 33 & 2.80 & 3.57 & $+0.78$ \\
 & GT  & 30 & 2.42 & 2.95 & $+0.54$ \\
 & GP  & 34 & 2.43 & 2.78 & $+0.34$ \\
 & MIX & 35 & 2.32 & 2.34 & $+0.02$ \\
\bottomrule
\end{tabular}
\end{table}
