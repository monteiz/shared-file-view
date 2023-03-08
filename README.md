# SharedFileView

> :warning: **This project is under construction**: Do not use it yet!

SharedFileView is a Node.js add-on that loads a file into a shared memory segment, allowing multiple processes to access the data quickly and efficiently, as a JavaScript array. The returned array is read-only.

SharedFileView is designed for speed and minimal memory usage. By using shared memory, SharedFileView eliminates the need to copy data between processes, resulting in blazing fast performance. Additionally, SharedFileView uses the minimum memory allocation possible to load the text file into memory, ensuring that your system resources are used efficiently.

## Requirements

Binaries are provided for OSX and Linux for various node versions
(check the releases page to see which). If a binary is not provided
for your platform, you will need Boost and and a C++11 compliant
compiler (like GCC 4.8 or better) to build the module.

## Installation

```npm
    npm install shared-file-view
```

## Unit tests

```npm
    npm test
```

## Publishing a binary release

To make a new binary release:

- Edit package.json. Increment the `version` property.
- `node-pre-gyp rebuild`
- `node-pre-gyp package`
- `node-pre-gyp-github publish`
- `npm publish`

You will need a `NODE_PRE_GYP_GITHUB_TOKEN` with `repo:status`,
`repo_deployment` and `public_repo` access to the target repo. You'll
also need write access to the npm repo.

## MSVS build prerequisites

Set up [Boost](http://www.boost.org/).

Set BOOST_ROOT environment variable.

```
bootstrap
b2 --build-type=complete
```

## License

SharedFileView is licensed under the [MIT License](https://opensource.org/licenses/MIT). See the [`LICENSE`](LICENSE) file for details.

## Credits

SharedFileView is built upon the work of several open-source projects and contributors. We would like to thank the following individuals and organizations for their contributions to SharedFileView:

- Seth Heeren ([@sehe](https://github.com/sehe)), for providing the original approach that underlie SharedFileView.
- Allen Luce ([@allenluce](https://github.com/allenluce)), for creating the [`mmap-object`](https://github.com/allenluce/mmap-object) package that inspired SharedFileView.

Without the the contributions of Seth Heeren and the pioneering work of Allen Luce, SharedFileView would not be possible. We are grateful for their efforts and for making their work available under the MIT License.
