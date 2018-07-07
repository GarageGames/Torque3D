Set-PSDebug -Trace 1

git config --global credential.helper store
add-content "$env:USERPROFILE\.git-credentials" "https://$($env:github_token):x-oauth-basic@github.com`n"
git config --global user.email $env:github_email
git config --global user.name "andijh92"

# copy right doc dir to gh-pages branch 
git add doc\$env:appveyor_repo_tag_name
git commit -m "added generated doc for pushing to gh-pages branch"
git branch tmp-doc
git checkout -q gh-pages
git checkout tmp-doc -- doc\$env:appveyor_repo_tag_name

git commit -m "added generated doc"
git push -q origin gh-pages
