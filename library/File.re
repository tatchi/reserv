let getFilenamesFromDirName = (dirname: string) => {
  let rec helper = (dirname: string) => {
    switch (Luv.File.Sync.opendir(dirname)) {
    | Error(e) =>
      Console.log("error while opening dir: " ++ Luv.Error.err_name(e));
      [||];
    | Ok(dir) =>
      switch (Luv.File.Sync.readdir(dir)) {
      | Error(e) =>
        Console.log("error while reading dir");
        [||];
      | Ok(dirents) =>
        Array.fold_left(
          (acc, dirent) => {
            let filePath =
              Filename.concat(dirname, Luv.File.Dirent.(dirent.name));
            switch (Luv.File.Dirent.(dirent.kind)) {
            | `FILE => Array.append([|filePath|], acc)
            | `DIR => Array.append(helper(filePath), acc)
            | _ => acc
            };
          },
          [||],
          dirents,
        )
      }
    };
  };
  helper(dirname);
};