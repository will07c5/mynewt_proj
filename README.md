## Overview

This is a skeleton project for building Mynewt apps.  

## Build

1. Download and install Apache Newt.

You will need to download the Apache Newt tool, as documented in the [Getting Started Guide](http://mynewt.apache.org/latest/get_started/native_install/index.html).

2. Download the Apache Mynewt & nimBLE Core package dependencies:

```no-highlight
    $ newt install
```

3. To build a given app target use:

```no-highlight
    $ newt build <target>
```

4. To build an image suitable for flashing a target use:

```no-highlight
    $  newt create-image <target> 1.0.0
```

## Flash

To flash the target use:

```no-highlight
    $  newt load <target>
```
