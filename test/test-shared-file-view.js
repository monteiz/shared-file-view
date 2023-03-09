"use strict";
/* global describe it beforeEach afterEach before after */
const binary = require("node-pre-gyp");
const path = require("path");
const sharedFileViewPath = binary.find(path.resolve(path.join(__dirname, "../package.json")));
const SharedFileView = require(sharedFileViewPath);
const expect = require("chai").expect;
const temp = require("temp");
const childProcess = require("child_process");
const fs = require("fs");
const os = require("os");
const which = require("which");
const async = require("async");

const methods = [
	"isOpen",
	"close",
	"valueOf",
	"toString",
	"close",
	"get_free_memory",
	"get_size",
	"bucket_count",
	"max_bucket_count",
	"load_factor",
	"max_load_factor",
	"propertyIsEnumerable",
];

const testFilePath = path.join(__dirname, "..", "testdata", "random_strings.txt");

describe("shared-file-view", function () {
	before(function () {
		temp.track();
		this.dir = temp.mkdirSync("node-shared");
	});

	describe("Static functions", function () {
		it("it creates", function (done) {
			SharedFileView.Create(testFilePath, (err) => {
				done(err);
			});
		});
		it("it exists", function () {
			expect(SharedFileView.Exists(testFilePath)).to.be.true;
		});
		it("it removes", function (done) {
			SharedFileView.Remove(testFilePath, (err) => {
				expect(SharedFileView.Exists(testFilePath)).to.be.false;
				done(err);
			});
		});
		it("it creates again", function (done) {
			SharedFileView.Create(testFilePath, (err) => {
				done(err);
			});
		});
	});

	describe("Instance functions", function () {
		it("get the array", function () {
			const array = new SharedFileView.ArrayFrom(testFilePath);
			expect(array.length).to.equal(10);
		});

		it("gets first line", function () {
			const array = new SharedFileView.ArrayFrom(testFilePath);
			expect(array[0]).to.equal("6wjZagsBmTxY3xeMc4brVplX");
		});
		it("gets second line", function () {
			const array = new SharedFileView.ArrayFrom(testFilePath);
			expect(array[1]).to.equal("bWw4FBK0iKnncEPDcz7weEMwZpxy2gkNdx3lc");
		});
		it("gets last line", function () {
			const array = new SharedFileView.ArrayFrom(testFilePath);
			expect(array[array.length - 1]).to.equal("mvkref3kq5VwOcCusoeOD2KJN6poRYFmN5WCr");
		});
		it("gets a line before allowed range", function () {
			const array = new SharedFileView.ArrayFrom(testFilePath);
			expect(array[-1]).to.be.undefined;
		});
		it("gets a line after allowed range", function () {
			const array = new SharedFileView.ArrayFrom(testFilePath);
			expect(array[array.length]).to.be.undefined;
		});
	});

	describe("Instance functions from main process", function () {
		it("works across processes", function (done) {
			process.env.TESTFILE = this.testfile;
			const child = childProcess.fork(which.sync("mocha"), ["./test/util-interprocess.js"]);
			child.on("exit", function (exitCode) {
				expect(child.signalCode).to.be.null;
				expect(exitCode, "error from util-interprocess.js").to.equal(0);
				done();
			});
		});
	});
});
