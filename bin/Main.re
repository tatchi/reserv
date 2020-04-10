let test = {
  Console.log("Hello, world!");

  switch (Luv.File.Sync.opendir("tatchi")) {
  | Error(_) => Console.log("error while opening dir")
  | Ok(dir) =>
    switch (Luv.File.Sync.readdir(dir)) {
    | Error(_) => Console.log("error while reading dir")
    | Ok(dirents) =>
      Array.iter(
        dirent => {
          let watcher = Luv.FS_poll.init() |> Result.get_ok;
          let w =
            Luv.FS_poll.start(
              ~interval=10,
              watcher,
              "tatchi/" ++ Luv.File.Dirent.(dirent.name),
              result => {
              switch (result) {
              | Error(e) => Console.log(Luv.Error.err_name(e))
              | Ok((fileStat1, fileStat2)) => Console.log("change")
              }
            });
          let threadFs = Luv.Thread.create(_ => w) |> Result.get_ok;
          ignore(Luv.Thread.join(threadFs));
        },
        dirents,
      )
    }
  };
  // Luv.FS_poll.start(
  //   ~interval=10,
  //   watcher,
  //   "index.html",
  //   fun
  //   | Error(e) => {
  //       Console.log("error");
  //     }
  //   | Ok((fileStat1, fileStat2)) => Console.log("change"),
  // );
};
ignore(Luv.Loop.run(test));