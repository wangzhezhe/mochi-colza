Thallium Microservice Template
==============================

This project is a template to start developing a Mochi microservice based on Thallium.
If you want to implement your own microservice, please read ahead. Though this project
provides many examples of how to use the Thallium API, you may want to refer to the Thallium
documentation [here](https://mochi.readthedocs.io/en/latest/) for more detail.


The Mochi philosophy and design overview
----------------------------------------

Please refer to the README file in 
[the margo template](https://xgitlab.cels.anl.gov/sds/templates/margo-microservice-template)
to have an overview of the Mochi philosophy and the design of a Mochi microservice.

Organization of this template project
-------------------------------------

This template project illustrates how a Thallium-based microservice could
be architectured. It can be compiled as-is, and provides a couple of
functionalities that make the provider print a "Hello World" message
on its standard output, or compute the sum of two integers.

This template project uses **alpha** as the name of your microservice.
Functions, types, files, and libraries therefore use the **alpha** prefix.
The first step in setting up this project for your microservice will be
to replace this prefix. The generic name **resource** should also be
replaced with a more specific name, such as **database**. This renaming
step can be done by using the _setup.py_ script at the root of this repository
(see next section).

The _include_ directory of this template project provides public header files.
* _alpha/Client.hpp_ contains the Client class;
* _alpha/ResourceHandle.hpp_ contains the ResourceHandle class, which provides
  functions to interact with a resource on a provider;
* _alpha/Provider.hpp_ contains the Provider class;
* _alpha/Backend.h_ contains the definition of an abstract clas that
  can be inherited from to implement new backends for the microservice.
* _alpha/Admin.h_ contains the Admin class, as well as admin functions to
  interact with a provider.

This template project uses the [pimpl idiom](https://en.cppreference.com/w/cpp/language/pimpl)
to hide the internal details of classes in implementation classes that are hidden from
users once compiled.
The implementation is located in the _src_ directory.

The _src/dummy_ directory provides a default implementation of a backend.
We recommend that you implement a dummy backend for your
service, as a way of testing application logic and RPCs without the burdon of complex
external dependencies. For instance, a dummy backend may be a backend that simply
acknowledges requests but does not process them, or provides mock results.

The project uses a number of dependencies:
* TCLAP for parsing program options in the examples;
* CppUnit for unit testing (in the tests directory);
* spdlog to provide logging.

The _examples_ directory contains an example using the microservice:
the server example will start one or more providers and print the server's address.
The admin example can connect to a provider and have it create a resource
(and print the resource id) or open, close, or destroy resources.
The client example can be run next to interact with the resource.

The template also contains a _spack.yaml_ file at its root that can be used to
install its dependencies. You may add additional dependencies into this file as
your microservice gets more complex.

As you modify this project to implement your own microservice, feel free to remove
any dependencies you don't like (such as TCLAP, spdlog, CppUnit) and adapt it to your needs!


Setting up your project
-----------------------

Let's assume you want to create a microservice called "yellow", which manages
a phone directory (association between names and phone numbers). The following
shows how to setup your project:

```
git clone https://xgitlab.cels.anl.gov/sds/templates/thallium-microservice-template.git
mv thallium-microservice-template yellow
cd yellow
rm -rf .git
python setup.py
$ Enter the name of your service: yellow
$ Enter the name of the resources (e.g., database): phonebook
```

The python script will edit and rename all the files, replacing _alpha_ with _yellow_
and _resource_ with _phonebook_ (with matching capitalization).

Building the project
--------------------

The project's dependencies may be build using [spack](https://spack.readthedocs.io/en/latest/).
You will need to have setup [sds-repo](https://xgitlab.cels.anl.gov/sds/sds-repo) as external
namespace for spack, which can be done as follows.

```
# from outside of your project directory
git clone git@xgitlab.cels.anl.gov:sds/sds-repo.git
spack repo add sds-repo
```

The easiest way to setup the dependencies for this project is to create a spack environment
using the _spack.yaml_ file located at the root of the project, as follows.

```
# create an anonymous environment
cd thallium-microservice-template
spack env activate .
spack install
```

or as follows.

```
# create an environment named myenv
cd thallium-microservice-template
spack env create myenv spack.yaml
spack env activate myenv
spack install
```

Once the dependencies have been installed, you may build the project as follows.

```
mkdir build
cd build
cmake .. -DENABLE_LOG_INFO=ON -DENABLE_LOG_ERROR=ON -DENABLE_LOG_DEBUG=ON -DENABLE_TESTS=ON -DENABLE_EXAMPLES=ON
make
```

You can test the project using `make test` from the build directory.
