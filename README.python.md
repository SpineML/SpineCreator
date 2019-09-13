Python in SpineCreator
======================

SpineCreator co-opts Python to allow the user to create their own
connection patterns. By default, SpineCreator compiles against Python
2.7 and links to the default python library on the system.

It is possible to use an alternative installation of Python. I had
need to do this because I wanted to use Numba/CUDA in some of my
connection functions. I therefore wanted to use Python 3.7 from
Anaconda, in which environment I had installed the CUDA tools and
Numba. I've updated SpineCreator with the ability to use an
alternative python. Here's what you need to do if you want to do
something similar:

## Compile SpineCreator against the correct libpython

First you need to compile SpineCreator against your python. In QtCreator,
open up spinecreator.pro. Edit the lines where python2.7 is linked and
replace with the relevant link for your python. For example, inside
the linux-g++-64 section, I changed two lines:

```
LIBS += -L/usr/lib/graphviz -L/opt/graphviz/lib -lGLU -lpython2.7
```
becomes
```
LIBS += -L/usr/lib/graphviz -L/opt/graphviz/lib -lGLU -L/home/seb/anaconda3/lib -lpython3.7m
```
and
```
INCLUDEPATH += /usr/include/python2.7
```
becomes
```
INCLUDEPATH += /home/seb/anaconda3/include/python3.7m
```

Now rebuild SpineCreator.

## Set up 'Python program path' and 'Python home'

Open SpineCreator. Go to Edit->Settings and choose the Python Scripts
tab in the window which should appear.

In the box for 'Python program path' put in the path to your
python. In my case, this is '/home/seb/anaconda3/bin/python'

In the box for 'Python home' put the right contents for PYTHONHOME for
your installed python. For my Anaconda installation, it's:

/home/seb/anaconda3:/home/seb/anaconda3/bin:/home/seb/anaconda3/lib:/home/seb/anaconda3/lib/python3.7:/home/seb/anaconda3/lib/python3.7/lib-dynload:/home/seb/anaconda3/lib/python3.7/site-packages:/home/seb/anaconda3/lib/python3.7/site-packages/numba:/home/seb/anaconda3/lib/python3.7/site-packages/numba/cuda

You can see there are some site packages listed there. Google up how
to find out what your PYTHONHOME is set to when you run your installed
python.

Now close SpineCreator.

## Run SpineCreator again, and check it launched first time

If you launch SpineCreator, and it doesn't appear; check the
Application Output window in QtCreator. It may tell you it
crashed. Perhaps you didn't compile SpineCreator against the correct
Python library.

If SpineCreator appears, congratulations. Now go and check the
settings window again and make sure that 'Python program path' and
'Python home' are still set to the expected values.

You should now find that your Python scripts in SpineCreator are
executed by the correct, non-standard Python installation.
