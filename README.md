Yet another text picture generator
===

This repo contains source code of simple yet powerfull text picture generator. It allows to render pictures with single line phrases with random:

 - font: family, size, border
 - geometry: rotation, scale, translation
 - color: background (linear and radial gradients are implemanted also), foreground
 - misc: noise, blur, multiply

Unicode symbols are also supported. Originally this utility was designed to create syntetic training dataset for the cirillyc + latin phrases in one the specific field of work. So, this tool produces output that is handy for [deep-text-recognition-benchmark](https://github.com/clovaai/deep-text-recognition-benchmark)    

**How to use**

It was designed as a console application, so to produce sample-pictures you should call it from the terminal:

```bash
textpictgen -t "phrase i want to render"
```
in output directory (./ by default) you will find textpictgen/ subfolder with samples:

![sample](https://github.com/pi-null-mezon/textpictgen/blob/master/samples/s1.png)

![sample](https://github.com/pi-null-mezon/textpictgen/blob/master/samples/s2.png)

![sample](https://github.com/pi-null-mezon/textpictgen/blob/master/samples/s3.png)



**Installation**

Generator is based on [Qt](https://www.qt.io/), so: 

```bash
cd textpictgen

mkdir build && cd build qmake ../textpictgen.pro

make && sudo make install
```

**LICENSE**

Be cool. Stay wild. No warranties are granted.
