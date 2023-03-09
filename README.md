# Shared File View

> :warning: **This project is under construction**: Do not use it yet!

SharedFileView is a Node.js add-on that loads a file into a shared memory segment, allowing multiple processes to access the data quickly and efficiently, as a JavaScript array. The returned array is read-only.

SharedFileView is designed for speed and minimal memory usage. By using shared memory, SharedFileView eliminates the need to copy data between processes, resulting in blazing fast performance. Additionally, SharedFileView uses the minimum memory allocation possible to load the text file into memory, ensuring that your system resources are used efficiently.

## Requirements

Boost libraries (^1.81) installed on the system. Installation will fail otherwise.

Follow [this guide](https://www.boost.org/doc/libs/1_81_0/more/getting_started/index.html) to install it.

## Installation

```npm
    npm install shared-file-view
```

## License

SharedFileView is licensed under the [MIT License](https://opensource.org/licenses/MIT). See the [`LICENSE`](LICENSE) file for details.

## Credits

SharedFileView is built upon the work of several open-source projects and contributors. We would like to thank the following individuals and organizations for their contributions to SharedFileView:

- Seth Heeren ([@sehe](https://github.com/sehe)), for providing the original approach that underlie SharedFileView.
- Allen Luce ([@allenluce](https://github.com/allenluce)), for creating the [`mmap-object`](https://github.com/allenluce/mmap-object) package that inspired SharedFileView.

Without the the contributions of Seth Heeren and the pioneering work of Allen Luce, SharedFileView would not be possible. We are grateful for their efforts and for making their work available under the MIT License.
