let test = {
  Console.log("Hello, world!");

  let getFiles = (dir: string) => {
    let rec helper = (dirString: string) => {
      switch (Luv.File.Sync.opendir(dirString)) {
      | Error(_) =>
        Console.log("error while opening dir");
        [||];
      | Ok(dir) =>
        switch (Luv.File.Sync.readdir(dir)) {
        | Error(_) =>
          Console.log("error while reading dir");
          [||];
        | Ok(dirents) =>
          Array.fold_left(
            (acc, dirent) =>
              switch (Luv.File.Dirent.(dirent.kind)) {
              | `FILE =>
                Array.append(
                  [|dirString ++ "/" ++ Luv.File.Dirent.(dirent.name)|],
                  acc,
                )
              | `DIR =>
                Array.append(
                  helper(dirString ++ "/" ++ Luv.File.Dirent.(dirent.name)),
                  acc,
                )
              | _ => acc
              },
            [||],
            dirents,
          )
        }
      };
    };

    helper(dir);
  };
  getFiles("build") |> Array.iter(Console.log);
};

ignore(Luv.Loop.run());