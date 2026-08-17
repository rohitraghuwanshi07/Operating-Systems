HOW TO USE THIS:

1. Take your existing title-page .tex file (the one with \input{preamble}
   and \tableofcontents \newpage).
2. Delete the final "\end{document}" line from it.
3. Paste the ENTIRE contents of report_body.tex right after that.
4. Place the code/ and images/ folders next to your main .tex file
   (same directory, or adjust paths in report_body.tex if you use
   subfolders differently).
5. Make sure your preamble.sty includes the `listings` and `graphicx`
   packages (needed for \lstinputlisting and \includegraphics). If not,
   add to preamble.sty:
     \usepackage{listings}
     \usepackage{graphicx}
     \usepackage{xcolor}
     \lstset{
       basicstyle=\ttfamily\small,
       breaklines=true,
       frame=single,
       numbers=left,
       numberstyle=\tiny,
     }
6. Compile with pdflatex (or Overleaf) twice (for the table of contents
   to resolve correctly).

Files included:
- code/q2_fork_skeleton.c, q3_runcmd.c, q4_shell.c  (final, debug-free)
- images/fig_q2_q3echo.png       -> Q2 output + Q3 echo hello
- images/fig_q3_sleep_usage.png  -> Q3 sleep 2 + usage error (cropped clean)
- images/fig_q4_final_clean.png  -> Q4 final background/foreground demo
- images/fig_q4_ps_proof.png     -> ps -ax proof of background process
