let getFilenamesFromDirName = (dirname: string) => {
  let (>>=) = Result.bind;

  let rec helper = (dirname: string) => {
    Luv.File.Sync.opendir(dirname)
    >>= Luv.File.Sync.readdir
    >>= Array.fold_left(
          (acc, dirent) => {
            let filePath =
              Filename.concat(dirname, Luv.File.Dirent.(dirent.name));
            switch (Luv.File.Dirent.(dirent.kind)) {
            | `FILE => acc |> Result.map(Array.append([|filePath|]))
            | `DIR =>
              helper(filePath)
              >>= (
                files => {
                  acc |> Result.map(Array.append(files));
                }
              )
            | _ => acc
            };
          },
          Ok([||]),
        );
  };
  helper(dirname);
};