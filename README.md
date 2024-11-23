# Sunflower Programming Language

Welcome to the official documentation for Sunflower, a programming language inspired by Python. Sunflower aims to provide a simple and expressive syntax while offering powerful features for building robust applications.

## Installation

Current installation procedures follow the following steps:

1) Download this github project
2) Build using cmake

For those using Visual Studio Code, launch this as a cmake project and hit build. Then run test to execute basic sunflower code in [tests/test.sf](tests/test.sf).

## Getting Started

While an executable for sunflower is still under development, feel free to view the sample test files in [tests](tests) directory.
Copy the code in [tests/test.sf](tests/test.sf) and run cmake tests to execute it.

## Sample Hello World Program

Here is a simple "Hello, World!" program written in Sunflower:

```sunflower
print("Hello, World!")
```

## Sample Tower of Hanoi Program

Here is a sample "Tower of Hanoi" program written in Sunflower:

```sunflower
fun tower_of_hanoi (n, source, auxiliary, target)
    if n == 1
        putln("Move disk 1 from " + source + " to " + target)
        return 0
    
    tower_of_hanoi(n - 1, source, target, auxiliary)
    put ("Move disk ")
    put (n)
    putln (" from " + source + " to " + target)
    tower_of_hanoi(n - 1, auxiliary, source, target)

n = 3
tower_of_hanoi(n, 'A', 'B', 'C')
```

## Features

Sunflower offers a wide range of features to make your programming experience enjoyable and efficient. Some of the key features include:

- Simple and readable syntax
- Dynamic typing
- Automatic memory management
- Object-oriented programming support
- Extensive standard library
- And much more!

For detailed information on each feature, refer to the official Sunflower documentation when it's out (it's currently not out)

## Resources

To learn more about Sunflower and explore its capabilities, check out the following resources:

<!-- - Official Sunflower website: [sunflowerlang.com](https://sunflowerlang.com) -->
- Sunflower GitHub repository: [github.com/csunflower](https://github.com/csunflower.git)
<!-- - Sunflower community forum: [forum.sunflowerlang.com](https://forum.sunflowerlang.com) -->

## Contributing

We welcome contributions from the Sunflower community. If you have any bug reports, feature requests, or code improvements, please submit them via the GitHub repository.

## License

Sunflower is released under the MIT License. See the [LICENSE](https://github.com/csunflower/LICENSE) file for more details.
